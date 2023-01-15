#include <sharpen/ProcessOps.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#include <TlHelp32.h>
#else
#include <sys/signal.h>
#endif

#include <sharpen/SystemError.hpp>

void sharpen::SuspendProcess(std::uint32_t processId)
{
#ifdef SHARPEN_IS_WIN
    HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,processId);
    if(snapshot == INVALID_HANDLE_VALUE)
    {
        sharpen::ThrowLastError();
    }
    THREADENTRY32 te;
    te.dwSize = static_cast<DWORD>(sizeof(te));
    if(::Thread32First(snapshot,&te) == TRUE)
    {
        do
        {
            if(te.th32OwnerProcessID == processId && te.dwSize >= FIELD_OFFSET(THREADENTRY32,th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
            {
                HANDLE thrd = ::OpenThread(THREAD_SUSPEND_RESUME,FALSE,te.th32ThreadID);
                if(thrd != nullptr)
                {
                    ::SuspendThread(thrd);
                    ::CloseHandle(thrd);
                }
            }
            te.dwSize = static_cast<DWORD>(sizeof(te));
        } while(Thread32Next(snapshot,&te));
    }
    ::CloseHandle(snapshot);
#else
    if(::kill(processId,SIGSTOP) == -1)
    {
        sharpen::ThrowLastError();
    }
#endif
}

void sharpen::ResumeProcess(std::uint32_t processId)
{
#ifdef SHARPEN_IS_WIN
    HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,processId);
    if(snapshot == INVALID_HANDLE_VALUE)
    {
        sharpen::ThrowLastError();
    }
    THREADENTRY32 te;
    te.dwSize = static_cast<DWORD>(sizeof(te));
    if(::Thread32First(snapshot,&te) == TRUE)
    {
        do
        {
            if(te.th32OwnerProcessID == processId && te.dwSize >= FIELD_OFFSET(THREADENTRY32,th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
            {
                HANDLE thrd = ::OpenThread(THREAD_SUSPEND_RESUME,FALSE,te.th32ThreadID);
                if(thrd != nullptr)
                {
                    ::ResumeThread(thrd);
                    ::CloseHandle(thrd);
                }
            }
            te.dwSize = static_cast<DWORD>(sizeof(te));
        } while(Thread32Next(snapshot,&te));
    }
    ::CloseHandle(snapshot);   
#else
    if(::kill(processId,SIGCONT) == -1)
    {
        sharpen::ThrowLastError();
    }
#endif
}