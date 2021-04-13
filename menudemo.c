/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FILE:    menudemo.c                                                       */
/*                                                                           */
/* PURPOSE: This example illustrates how to use the Menu Utility instrument  */
/*          driver to implement Most Recently Used (MRU) sections on CVI     */
/*          menus.  This particular example implements MRU lists for the File*/
/*          and Window menus.  The MRU lists can be stored in the Inifle     */
/*          objects, provided by the Inifile instrument driver.              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/*---------------------------------------------------------------------------*/
#include <cvirte.h>
#include <userint.h>
#include "menudemo.h"
#include "menuutil.h"

/*---------------------------------------------------------------------------*/
/* Defines                                                                   */
/*---------------------------------------------------------------------------*/
#ifdef WIN32
  #define DEMO_REGISTRY_NAME "Software\\National Instruments\\CVI Sample\\MenuDemo"
#else
  #define DEMO_REGISTRY_NAME "menudemo.ini"
#endif

/*---------------------------------------------------------------------------*/
/* Types                                                                     */
/*---------------------------------------------------------------------------*/
typedef struct WindowMenuCallbackDataRec_Tag
{
    char fileName[MAX_PATHNAME_LEN];
    int  panel;
} WindowMenuCallbackData;

/*---------------------------------------------------------------------------*/
/* Module-globals                                                            */
/*---------------------------------------------------------------------------*/
static int g_panelHandle;
static int g_menubarHandle;
static IniText g_iniTextHandle = 0;
static menuList g_fileMenuListHandle = 0;
static menuList g_winMenuListHandle = 0;
static char g_msgBuffer[512];
static int g_numChildPanels = 0;
static int g_topLeftValue = 0;

/*---------------------------------------------------------------------------*/
/* Internal function prototypes                                              */
/*---------------------------------------------------------------------------*/
static int DimMenuItems                (void);
static int GetTopChildWindow           (int panelHandle);
static int SaveOptionsForUIR           (void);
static int GetOptionsForUIR            (void);
static int CreateWindowMenuList        (void); 
static int RemoveWindowMenuList        (void); 
static int AddFileNameToWindowMenuList (menuList menuListHandle,
                                        char *fullPathAndFilename, int panel);
static int AddFileNameToFileMenuList   (menuList menuListHandle,
                                        char *fullPathAndFilename);
static int RemoveFileNameFromMenuList  (menuList menuListHandle,
                                        char *fullPathAndFilename);
static int CheckForFileNameInMenuList  (menuList menuListHandle,
                                        char *fullPathAndFilename);

static void CVICALLBACK FILEMenuListCallbackFunc (menuList list, int menuIndex,
                                                  int event,
                                                  void *callbackData);
static void CVICALLBACK WINDOWMenuListCallbackFunc (menuList list,
                                                    int menuIndex, int event,
                                                    void *callbackData);

/*---------------------------------------------------------------------------*/
/* This is the application's entry-point.                                    */
/*---------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;
    if ((g_panelHandle = LoadPanel (0, "menudemo.uir", PANEL)) < 0)
        return -1;
    g_menubarHandle = GetPanelMenuBar (g_panelHandle);
    GetOptionsForUIR ();
    DimMenuItems ();
    CreateWindowMenuList ();
    DisplayPanel (g_panelHandle);
    RunUserInterface ();
    
    /* Free resources and return */
    RemoveWindowMenuList ();
    SaveOptionsForUIR ();
    DiscardPanel (g_panelHandle);
    CloseCVIRTE ();
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Dim File and Window menu items depending on whether or not a file is      */
/* open.                                                                  */ 
/*---------------------------------------------------------------------------*/
static int DimMenuItems(void) 
{
    SetMenuBarAttribute (g_menubarHandle, MAINMENU_FILE_SAVE, ATTR_DIMMED,
                         !g_numChildPanels);
    SetMenuBarAttribute (g_menubarHandle, MAINMENU_FILE_SAVEAS, ATTR_DIMMED,
                         !g_numChildPanels);
    SetMenuBarAttribute (g_menubarHandle, MAINMENU_FILE_CLOSE, ATTR_DIMMED,
                         !g_numChildPanels);
    SetMenuBarAttribute (g_menubarHandle, MAINMENU_WINDOW_CLOSEALL,
                         ATTR_DIMMED, !g_numChildPanels);
    SetMenuBarAttribute (g_menubarHandle, MAINMENU_WINDOW_HIDEALL,
                         ATTR_DIMMED, !g_numChildPanels);
    return 0;    
}        

/*---------------------------------------------------------------------------*/
/* Create a menu list in the WINDOW menu.                                    */
/*---------------------------------------------------------------------------*/
static int CreateWindowMenuList (void) 
{
    if (!g_winMenuListHandle)
        g_winMenuListHandle = MU_CreateMenuList (g_menubarHandle,
                                                 MAINMENU_WINDOW, -1, 5,
                                                 WINDOWMenuListCallbackFunc);
        
    if (g_winMenuListHandle) 
        {
        
        /* Update Attributes of the successfully createed list*/
        MU_SetMenuListAttribute (g_winMenuListHandle, 0,
                                 ATTR_MENULIST_APPEND_SHORTCUT, 1);
        MU_SetMenuListAttribute (g_winMenuListHandle, 0,
                                 ATTR_MENULIST_ALLOW_DUPLICATE_ITEMS, 0);
        }    
    else
        MessagePopup ("MenuDemo", "Unable to create WINDOW menu list.");
    return 0;
}

/*--------------------------------------------------------------------------*/
/* Remove menu list from WINDOW menu.                                       */
/*--------------------------------------------------------------------------*/
static int RemoveWindowMenuList(void) 
{
    if (g_fileMenuListHandle)
        MU_DeleteMenuList (g_winMenuListHandle);
    return 0;        
}

/*---------------------------------------------------------------------------*/
/* Get saved UIR options and update the UIR -- this means initializing the   */
/* MRU File menu item list.                                                  */
/*---------------------------------------------------------------------------*/
static int GetOptionsForUIR (void)
{
    int success = 1;

    /* Create an INI object */
    if (!g_iniTextHandle)
        g_iniTextHandle = Ini_New (0);

    /* Get options and update UIR */
    if (g_iniTextHandle)
        {
        
        /* Read previous MRU list data from system */      
        MU_ReadRegistryInfo (g_iniTextHandle, DEMO_REGISTRY_NAME);

        /* Create FILE menulist if it does not already exist */
        if (!g_fileMenuListHandle)
            g_fileMenuListHandle = MU_CreateMenuList (g_menubarHandle,
                                                      MAINMENU_FILE,
                                                      MAINMENU_FILE_ABOVE_EXIT_LINE,
                                                      5,
                                                      FILEMenuListCallbackFunc);
            
        if (g_fileMenuListHandle)
            {
            
            /* Update attributes */
            MU_SetMenuListAttribute (g_fileMenuListHandle, 0,
                                     ATTR_MENULIST_APPEND_SHORTCUT, 1);
            MU_SetMenuListAttribute (g_fileMenuListHandle, 0,
                                     ATTR_MENULIST_ALLOW_DUPLICATE_ITEMS,
                                     0);
            
            /* Update FILE menulist with files from INI object */
            MU_GetFileListFromIniFile (g_fileMenuListHandle, g_iniTextHandle,
                                       "FILE MenuList", "Filename", 1);
            }    
        }
    else
        success = 0;
    return success;
}   

/*---------------------------------------------------------------------------*/
/* Save options from current state of UIR to the system.  Thius means storing*/
/* the MRU list on the File menu.                                            */
/*---------------------------------------------------------------------------*/
static int SaveOptionsForUIR (void)
{
    int success = 1;
    
    if (g_iniTextHandle)
        {
        
        /* Save FILE menulist filenames to INI object */
        if (g_fileMenuListHandle)
            MU_PutFileListInIniFile (g_fileMenuListHandle, g_iniTextHandle,
                                     "FILE MenuList", "Filename", TRUE);
            
        /* Destroy FILE menulist */      
        if (g_fileMenuListHandle)
             MU_DeleteMenuList (g_fileMenuListHandle);
             
        /* Save options from UIR to system */
        if (!MU_WriteRegistryInfo (g_iniTextHandle, DEMO_REGISTRY_NAME))
            success = 0;
        }   
    else
        success = 0;   
    
    /* Discard INI object */    
    if (g_iniTextHandle)
        Ini_Dispose (g_iniTextHandle);
    return success;
}   

/*--------------------------------------------------------------------------*/
/* Add a particular file name to the File menu's MRU list.                  */
/*--------------------------------------------------------------------------*/
static int AddFileNameToFileMenuList (menuList menuListHandle,
                                      char *fullPathAndFilename)
{
    int retVal = 0;
    
    if ((menuListHandle) && (fullPathAndFilename)
        &&  (fullPathAndFilename[0] != '\0') )
        retVal = MU_AddItemToMenuList (menuListHandle, FRONT_OF_LIST,
                                       MU_MakeShortFileName (NULL,
                                                             fullPathAndFilename,
                                                             32),
                                       StrDup (fullPathAndFilename));
    return retVal;   
}       

/*--------------------------------------------------------------------------*/
/* Add a particular window entry to the Window menu's MRU list.             */
/*--------------------------------------------------------------------------*/
static int AddFileNameToWindowMenuList (menuList menuListHandle,
                                        char *fullPathAndFilename,
                                        int panel)
{
    int                    retVal = 0;
    WindowMenuCallbackData *callbackData;
    
    if ((menuListHandle) && (fullPathAndFilename)
        &&  (fullPathAndFilename[0] != '\0')) 
        { 
        if ((callbackData = calloc (1, sizeof(WindowMenuCallbackData))))
            {
            strcpy (callbackData->fileName, fullPathAndFilename);
            callbackData->panel = panel;
            retVal = MU_AddItemToMenuList (menuListHandle, FRONT_OF_LIST,
                                           MU_MakeShortFileName (NULL,
                                                                 fullPathAndFilename,
                                                                 32),
                                           callbackData);
            }    
        }        
    return retVal;   
}       

/*---------------------------------------------------------------------------*/
/* Remove a particular file from the File menu's MRU list.                   */
/*---------------------------------------------------------------------------*/
static int RemoveFileNameFromMenuList (menuList menuListHandle,
                                       char *fullPathAndFilename)
{
    int  retVal = 0;
    int  item = 0;
    int  totalItems = 0;
    char *fileName;
    
    /* Find menu item and remove from list */
    if ((menuListHandle) && (fullPathAndFilename)
        &&  (fullPathAndFilename[0] != '\0')) 
        { 
        if ((totalItems = MU_GetNumMenuListItems (menuListHandle)))
            {
            for (item = 1; item <= totalItems; item++) 
                {
                fileName = 0;
                MU_GetMenuListAttribute (menuListHandle, item,
                                         ATTR_MENULIST_ITEM_CALLBACK_DATA,
                                         &fileName);
                if ((fileName) &&
                    (strcmp (fileName, fullPathAndFilename) == 0))
                    break;
                }    
            if (item <= totalItems)
                MU_DeleteMenuListItem (menuListHandle, item);
            }    
        }        
    return retVal;   
}       

/*---------------------------------------------------------------------------*/
/* Check to see if the file is already in the Window Menu MRU list.          */
/*---------------------------------------------------------------------------*/
static int CheckForFileNameInMenuList (menuList menuListHandle,
                                       char *fullPathAndFilename)
{
    int  retVal = 0;
    int  item = 0;
    int  totalItems = 0;
    char *fileName;
    
    /* Find menu item in list*/
    if ((menuListHandle) && (fullPathAndFilename)
        &&  (fullPathAndFilename[0] != '\0')) 
        { 
        if ((totalItems = MU_GetNumMenuListItems (menuListHandle)))
            {
            for (item = 1; item <= totalItems; item++) 
                {
                fileName = 0;
                MU_GetMenuListAttribute (menuListHandle, item,
                                         ATTR_MENULIST_ITEM_CALLBACK_DATA,
                                         &fileName);
                if ((fileName) &&
                    (strcmp (fileName, fullPathAndFilename) == 0))
                    retVal = 1;
                }
            }
        }        
    return retVal;   
}       

/*---------------------------------------------------------------------------*/
/* This menu callback function is called when a filename is chosen from the  */
/* MRU list on the File menu.  callbackData will point to the long filename, */
/* since we're just using the short one in the MRU list.                     */
/*---------------------------------------------------------------------------*/
static void CVICALLBACK FILEMenuListCallbackFunc (menuList list, int menuIndex,
                                                  int event,
                                                  void *callbackData)
{
    int  status;
    int  childPanel;
    char *fileName = (char *)callbackData;
   
    if (event == EVENT_DISCARD)
        {
        if (fileName)
            free (fileName);
        }
    else if (fileName) 
        {
        if (!CheckForFileNameInMenuList(g_winMenuListHandle, fileName))
            {
            /* Create a child panel to represent the newly opened file */
            if ((childPanel = LoadPanel (g_panelHandle, "menudemo.uir", FILEPANEL))
                >= 0)
                {
            
                /* Set top and left for child panel */
                SetPanelAttribute (childPanel, ATTR_TOP,
                                   g_topLeftValue * 25 + 50);
                SetPanelAttribute (childPanel, ATTR_LEFT,
                                   g_topLeftValue * 25 + 25);
                g_topLeftValue = (g_topLeftValue + 1) % 5;
            
                /* Set Title of child panel */
                SetPanelAttribute (childPanel, ATTR_TITLE,
                                   MU_MakeShortFileName (NULL, fileName, 32));
                SetCtrlVal (childPanel, FILEPANEL_FILENAME, fileName);
            
                /* Display child panel */
                DisplayPanel (childPanel);
                g_numChildPanels++;
            
                /* Add an item for this window to the WINDOW MRU list */
                status = AddFileNameToWindowMenuList (g_winMenuListHandle,
                                                      fileName, childPanel);
                DimMenuItems ();
                }
            else
                {
                sprintf (g_msgBuffer, "Failure in LoadPanel.");
                MessagePopup ("MenuDemo", g_msgBuffer);
                }
            }
        else
            MessagePopup("File Open", "File is already open.  Use Window menu to see list of open files.");
        }   
    return;         
}

/*---------------------------------------------------------------------------*/
/* This menu callback function is called when a filename is chosen in the    */
/* WINDOW menu's MRU list.  callbackData will point to a structure we use    */
/* to keep track of the item data.                                           */
/*---------------------------------------------------------------------------*/
static void CVICALLBACK WINDOWMenuListCallbackFunc (menuList list,
                                                    int menuIndex, int event,
                                                    void *callbackData)
{
    WindowMenuCallbackData *data = callbackData;
   
    if (event == EVENT_DISCARD)
        {
        if (data)
            free(data);
        }
    else
        {
        
        /* Restore the window and bring to focus */
        if (data) 
            DisplayPanel (data->panel);
        }   
    return;         
}

/*---------------------------------------------------------------------------*/
/* MainPanelCallback                                                         */
/*---------------------------------------------------------------------------*/
int CVICALLBACK MainPanelCallback (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_CLOSE:
            FileQuit (0, 0, NULL, panel);
            break;
        }
    return 0;
}


/*---------------------------------------------------------------------------*/
/* Respond to the File->Open command to open a new file, which in this demo  */
/* basically just creates a child panel for it.                              */
/*---------------------------------------------------------------------------*/
void CVICALLBACK FileOpen (int menuBar, int menuItem, void *callbackData,
                          int panel)
{
    int  stat;
    int  childPanel;
    char fileName[MAX_PATHNAME_LEN];
    
    /* Get fileName from user */    
    stat = FileSelectPopupEx ("", "*.*", "", "Choose a file to open",
							  VAL_LOAD_BUTTON, 0, 0, fileName);
    if ((stat == VAL_EXISTING_FILE_SELECTED)
        || (stat == VAL_NEW_FILE_SELECTED))
        {
        if (!CheckForFileNameInMenuList(g_winMenuListHandle, fileName))
            {
            /* Create a child panel to represent the newly opened file */
            if ((childPanel = LoadPanel (panel, "menudemo.uir", FILEPANEL))
                >= 0)
                {
            
                /* Set top and left for child panel */
                SetPanelAttribute (childPanel, ATTR_TOP,
                                   g_topLeftValue * 25 + 50);
                SetPanelAttribute (childPanel, ATTR_LEFT,
                                   g_topLeftValue * 25 + 25);
                g_topLeftValue = (g_topLeftValue + 1) % 5;
            
                /* Set Title of child panel */
                SetPanelAttribute (childPanel, ATTR_TITLE,
                                   MU_MakeShortFileName(NULL, fileName, 32));
                SetCtrlVal (childPanel, FILEPANEL_FILENAME, fileName);
            
                /* Display the child panel */
                DisplayPanel (childPanel);
                g_numChildPanels++;
            
                /* Add a menu item to the WINDOW list for this opened window */
                AddFileNameToWindowMenuList (g_winMenuListHandle, fileName,
                                             childPanel);
    
                DimMenuItems ();
                sprintf (g_msgBuffer, "You selected to 'open' the following file:"
                                      "\n  %s\n\nNow save or close the file to add"
                                      " it to the 'File' menu.", fileName);
                }
            else
                sprintf (g_msgBuffer, "Failure to LoadPanel.");
            MessagePopup("Menu Utility Demo",g_msgBuffer);
            }
        else
            MessagePopup("File Open", "File is already open.  Use Window menu to see list of open files.");
        }
}

/*----------------------------------------------------------------------------*/
/* Respond to the File->Save As menu item to save a currently open file, which*/
/* includes adding it to the FILE menu's MRU list.                            */
/*----------------------------------------------------------------------------*/
void CVICALLBACK FileSaveAs (int menuBar, int menuItem, void *callbackData,
                             int panel)
{
    int  stat;
    int  childPanel;
    char fileName[MAX_PATHNAME_LEN];
    char oldFileName[MAX_PATHNAME_LEN];
        
    childPanel = GetTopChildWindow (panel);    
    if ((childPanel != 0)
        && (GetCtrlVal (childPanel, FILEPANEL_FILENAME, oldFileName) == 0))
        {
        stat = FileSelectPopupEx ("", "*.*", "", "Choose a filename to save as:",
								  VAL_SAVE_BUTTON, 0, 0, fileName);
                              
        if ((stat == VAL_EXISTING_FILE_SELECTED)
            || (stat == VAL_NEW_FILE_SELECTED))
            {
            
            /* Add menu item to FILE list */
            AddFileNameToFileMenuList (g_fileMenuListHandle, oldFileName);
            
            /* Remove old menu item from WINDOW list */
            RemoveFileNameFromMenuList (g_winMenuListHandle, oldFileName);
            
            /* Add menu item to WINDOW list */
            SetPanelAttribute (childPanel, ATTR_TITLE,
                               MU_MakeShortFileName(NULL, fileName, 32));
            SetCtrlVal (childPanel, FILEPANEL_FILENAME, fileName);
            AddFileNameToWindowMenuList (g_winMenuListHandle, fileName,
                                         childPanel);
            DimMenuItems ();
            sprintf (g_msgBuffer, "The following file will be saved:"
                                  "\n  %s\n\nThe old filename will be added "
                                  "to the 'File' menu's MRU list\n\n"
                                  "NOTE:  The actual file will not be modified.", fileName);
            MessagePopup ("MenuDemo", g_msgBuffer);
            }    
        }    
}

/*---------------------------------------------------------------------------*/
/* Respond to the File->Save menu option to save the currently open file.    */
/* This includes adding it to the File menu's MRU list.                      */
/*---------------------------------------------------------------------------*/
void CVICALLBACK FileSave (int menuBar, int menuItem, void *callbackData,
                           int panel)
{
    int  childPanel;
    char fileName[MAX_PATHNAME_LEN];
        
    childPanel = GetTopChildWindow (panel);    
    if ((childPanel != 0)
        && (GetCtrlVal (childPanel, FILEPANEL_FILENAME, fileName) == 0))
        {
        /* Add menu item to FILE list */
        AddFileNameToFileMenuList (g_fileMenuListHandle, fileName);
        sprintf (g_msgBuffer, "The following file was selected to be saved:\n"
                              " %s\n\nNOTE: Actual file will not be modified.",
                              fileName);
        MessagePopup ("MenuDemo",g_msgBuffer);
        }							 
}

/*----------------------------------------------------------------------------*/
/* Get the topmost child panel for a given parent.                            */
/*----------------------------------------------------------------------------*/
static int GetTopChildWindow (int panelHandle)
{
    int i;
    int panel = 0;
    int childPanels = 0;
    int lowZPlane = 0, lowPanel = 0, zPlane;
    
    GetPanelAttribute (panelHandle, ATTR_NUM_CHILDREN, &childPanels);

    /* Find lowest ZPLANE child window */
    for (i=0;i<childPanels;i++)
        {
        if (i == 0)
            {
            GetPanelAttribute (panelHandle, ATTR_FIRST_CHILD, &panel);
            GetPanelAttribute (panel, ATTR_ZPLANE_POSITION, &zPlane);
            lowZPlane = zPlane;
            lowPanel = panel;
            }    
        else 
            {
            GetPanelAttribute (panel, ATTR_NEXT_PANEL, &panel);
            if (panel) 
                {
                GetPanelAttribute (panel, ATTR_ZPLANE_POSITION, &zPlane);
                if (zPlane<lowZPlane) 
                    {
                    lowZPlane = zPlane;
                    lowPanel = panel;
                    }    
                }    
            }    
        }  
    if (childPanels)
        return lowPanel;
    else
        return -1;
}

/*----------------------------------------------------------------------------*/
/* Respond to File->Close menu item to close a file and add it to the MRU list*/                                                                  
/*----------------------------------------------------------------------------*/
void CVICALLBACK FileClose (int menuBar, int menuItem, void *callbackData,
                            int panel)
{
    char fileName[MAX_PATHNAME_LEN];
    
    if ((panel = GetTopChildWindow(panel)) >= 0)
        {    
        GetCtrlVal (panel, FILEPANEL_FILENAME, fileName);
        DiscardPanel (panel);
        AddFileNameToFileMenuList (g_fileMenuListHandle, fileName);
        RemoveFileNameFromMenuList (g_winMenuListHandle, fileName);
        g_numChildPanels--;
        DimMenuItems ();
        }    
}

/*---------------------------------------------------------------------------*/
/* Respond to File->Exit menu item to close the app.  We must add currently  */
/* open files to the File menu's MRU list, and remove items from the Window  */
/* menu's MRU list.                                                          */
/*---------------------------------------------------------------------------*/
void CVICALLBACK FileQuit (int menuBar, int menuItem, void *callbackData,
                           int panel)
{
    char *fileName;
    
    if (g_numChildPanels)
        {
        while ((MU_GetMenuListAttribute (g_winMenuListHandle, 1,
                                         ATTR_MENULIST_ITEM_CALLBACK_DATA,
                                         &fileName) == 0))
            {
            AddFileNameToFileMenuList (g_fileMenuListHandle, fileName);
            RemoveFileNameFromMenuList (g_winMenuListHandle, fileName);
            }
        sprintf (g_msgBuffer, "You are now exiting, any open files will be "
                              "saved in the File list");
        MessagePopup("MenuDemo", g_msgBuffer);
        g_numChildPanels = 0;
        DimMenuItems ();
    }    
    QuitUserInterface (0);
}

/*--------------------------------------------------------------------------*/
/* Hide all of the currently open child windows.                            */
/*--------------------------------------------------------------------------*/
void CVICALLBACK HideAllWindows (int menuBar, int menuItem,
                                 void *callbackData, int panel)
{
    int childPanels = 0;
    int childPanelHandle = 0;
    int i;
        
    GetPanelAttribute (panel, ATTR_NUM_CHILDREN, &childPanels);
    for (i = 0; i < childPanels; i++)
        {
        if (i == 0)
            GetPanelAttribute (panel, ATTR_FIRST_CHILD, &childPanelHandle);
        else 
            GetPanelAttribute (childPanelHandle, ATTR_NEXT_PANEL, &childPanelHandle);
        if (childPanelHandle)
            SetPanelAttribute (childPanelHandle, ATTR_VISIBLE, 0);
        }    
}

/*--------------------------------------------------------------------------*/
/* Close all of the currently open windows.                                 */
/*--------------------------------------------------------------------------*/
void CVICALLBACK CloseAllWindows (int menuBar, int menuItem,
                                  void *callbackData, int panel)
{
    int  childPanels = 0;
    int  i;
    int  childPanel;
    char fileName[MAX_PATHNAME_LEN];
    
    GetPanelAttribute (panel, ATTR_NUM_CHILDREN, &childPanels);
    for (i = 0; i < childPanels; i++) 
        {
        childPanel = GetTopChildWindow (panel);    
        GetCtrlVal (childPanel, FILEPANEL_FILENAME, fileName);
        AddFileNameToFileMenuList (g_fileMenuListHandle, fileName);
        RemoveFileNameFromMenuList (g_winMenuListHandle, fileName);
        DiscardPanel (childPanel);
        g_numChildPanels--;
        }    
    DimMenuItems ();
}

/*----------------------------------------------------------------------------*/
/* Help                                                                       */
/*----------------------------------------------------------------------------*/
void CVICALLBACK Help (int menubar, int menuItem, void *callbackData, int panel)
{
    MessagePopup ("MenuDemo","This demo illustrates the use of the Menu Util\n"
                             "Instrument Driver to implement MRU lists on CVI"
                             "\nmenus.  See the source file and FP for more "
                             "information.");
}



