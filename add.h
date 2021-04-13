#ifndef __MYDLL_H__
#define __MYDLL_H__

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) int __stdcall add(int x);
#ifdef __cplusplus
}
#endif

#endif // __MYDLL_H__