#if (defined(_WIN32)) || (defined(_WIN64))


#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <sharpen/WinEx.h>
#include <stdint.h>
#include <stdio.h>


BOOL CreatePipeEx(OUT LPHANDLE lpReadPipe,
                  OUT LPHANDLE lpWritePipe,
                  IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
                  IN DWORD nSize,
                  DWORD dwReadMode,
                  DWORD dwWriteMode) {
    static volatile long PipeSerialNumber = 0;

    HANDLE ReadPipeHandle, WritePipeHandle;
    DWORD dwError;
    char PipeNameBuffer[MAX_PATH];
    if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (nSize == 0) {
        nSize = 4096;
    }
    snprintf(PipeNameBuffer,
             sizeof(PipeNameBuffer),
             "\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
             (uint32_t)GetCurrentProcessId(),
             (uint32_t)InterlockedIncrement(&PipeSerialNumber));
    ReadPipeHandle = CreateNamedPipeA(PipeNameBuffer,
                                      PIPE_ACCESS_INBOUND | dwReadMode,
                                      PIPE_TYPE_BYTE | PIPE_WAIT,
                                      1,
                                      nSize,
                                      nSize,
                                      120 * 1000,
                                      lpPipeAttributes);
    if (!ReadPipeHandle) {
        return FALSE;
    }
    WritePipeHandle = CreateFileA(PipeNameBuffer,
                                  GENERIC_WRITE,
                                  0,
                                  lpPipeAttributes,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | dwWriteMode,
                                  NULL);
    if (INVALID_HANDLE_VALUE == WritePipeHandle) {
        dwError = GetLastError();
        CloseHandle(ReadPipeHandle);
        SetLastError(dwError);
        return FALSE;
    }

    *lpReadPipe = ReadPipeHandle;
    *lpWritePipe = WritePipeHandle;
    return (TRUE);
}

#else

typedef int MakeIsoCompilersHappy;

#endif