#pragma once
#ifndef _SHARPEN_WINEX_H
#define _SHARPEN_WINEX_H

#ifdef __cplusplus
extern "C"
{
#endif
    extern BOOL CreatePipeEx(OUT LPHANDLE lpReadPipe,OUT LPHANDLE lpWritePipe,IN LPSECURITY_ATTRIBUTES lpPipeAttributes,IN DWORD nSize,DWORD dwReadMode,DWORD dwWriteMode);
#ifdef __cplusplus
}
#endif

#endif