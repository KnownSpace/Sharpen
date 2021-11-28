#include <sharpen/FileOps.hpp>

#include <sharpen/SystemMacro.hpp>

#ifdef SHARPEN_IS_WIN
#include <io.h>
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <sharpen/SystemError.hpp>

bool sharpen::AccessFile(const char *name,sharpen::FileAccessModel model)
{
    int mod = 0;
#ifdef SHARPEN_IS_WIN
    switch (model)
    {
    case sharpen::FileAccessModel::Read:
        mod = 4;
        break;
    case sharpen::FileAccessModel::Write:
        mod = 2;
        break;
    case sharpen::FileAccessModel::All:
        mod = 6;
    default:
        break;
    }
    return _access_s(name,mod) == 0;
#else
    switch (model)
    {
    case sharpen::FileAccessModel::Read:
        mod = R_OK;
        break;
    case sharpen::FileAccessModel::Write:
        mod = W_OK;
        break;
    case sharpen::FileAccessModel::All:
        mod = R_OK | W_OK;
    default:
        break;
    }
    return ::access(name,mod) == 0;
#endif
}

bool sharpen::ExistFile(const char *name)
{
#ifdef SHARPEN_IS_WIN
    return ::_access_s(name,0) == 0;
#else
    return ::access(name,F_OK) == 0;
#endif
}

void sharpen::RenameFile(const char *oldName,const char *newName)
{
#ifdef SHARPEN_IS_WIN
    if(::MoveFileExA(oldName,newName,MOVEFILE_COPY_ALLOWED) == FALSE)
    {
        sharpen::ThrowLastError();
    }
#else
    if(::rename(oldName,newName) == -1)
    {
        sharpen::ThrowLastError();
    }
#endif
}

void sharpen::RemoveFile(const char *name)
{
#ifdef SHARPEN_IS_WIN
    if(::DeleteFileA(name) == FALSE)
    {
        sharpen::ThrowLastError();
    }
#else
    if(::remove(name) == -1)
    {
        sharpen::ThrowLastError();
    }
#endif
}