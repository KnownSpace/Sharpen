#pragma once
#ifndef _SHARPEN_WINEX_HPP
#define _SHARPEN_WINEX_HPP

#ifdef __cplusplus
extern "C"
{
#endif
    extern BOOL CreatePipeEx(OUT LPHANDLE lpReadPipe,OUT LPHANDLE lpWritePipe,IN LPSECURITY_ATTRIBUTES lpPipeAttributes,IN DWORD nSize,DWORD dwReadMode,DWORD dwWriteMode);
#ifdef __cplusplus
}
#endif

#endif