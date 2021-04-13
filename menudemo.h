/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  FILEPANEL                        1
#define  FILEPANEL_TEXTMSG                2       /* control type: textMsg, callback function: (none) */
#define  FILEPANEL_FILENAME               3       /* control type: textMsg, callback function: (none) */

#define  PANEL                            2       /* callback function: MainPanelCallback */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MAINMENU                         1
#define  MAINMENU_FILE                    2
#define  MAINMENU_FILE_OPEN               3       /* callback function: FileOpen */
#define  MAINMENU_FILE_SAVE               4       /* callback function: FileSave */
#define  MAINMENU_FILE_SAVEAS             5       /* callback function: FileSaveAs */
#define  MAINMENU_FILE_CLOSE              6       /* callback function: FileClose */
#define  MAINMENU_FILE_TOP_SEPARATOR      7
#define  MAINMENU_FILE_ABOVE_EXIT_LINE    8
#define  MAINMENU_FILE_QUIT               9       /* callback function: FileQuit */
#define  MAINMENU_EDIT                    10
#define  MAINMENU_EDIT_UNDO               11
#define  MAINMENU_EDIT_REDO               12
#define  MAINMENU_EDIT_SEPARATOR          13
#define  MAINMENU_EDIT_CUT                14
#define  MAINMENU_EDIT_COPY               15
#define  MAINMENU_EDIT_PASTE              16
#define  MAINMENU_EDIT_SEPARATOR_2        17
#define  MAINMENU_EDIT_FIND               18
#define  MAINMENU_EDIT_REPLACE            19
#define  MAINMENU_VIEW                    20
#define  MAINMENU_VIEW_TAB                21
#define  MAINMENU_WINDOW                  22
#define  MAINMENU_WINDOW_HIDEALL          23      /* callback function: HideAllWindows */
#define  MAINMENU_WINDOW_CLOSEALL         24      /* callback function: CloseAllWindows */
#define  MAINMENU_SEQUENCE                25
#define  MAINMENU_SEQUENCE_ADD            26
#define  MAINMENU_SEQUENCE_COMBINATE      27
#define  MAINMENU_RUN                     28
#define  MAINMENU_RUN_STEP                29
#define  MAINMENU_RUN_ALL                 30
#define  MAINMENU_HELP                    31      /* callback function: Help */
#define  MAINMENU_HELP_ABOUT              32
#define  MAINMENU_HELP_HANDBOOK           33


     /* Callback Prototypes: */

void CVICALLBACK CloseAllWindows(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK FileClose(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK FileOpen(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK FileQuit(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK FileSave(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK FileSaveAs(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK Help(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK HideAllWindows(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK MainPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
