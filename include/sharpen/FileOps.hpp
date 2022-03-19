#pragma once
#ifndef _SHARPEN_FILEOPS_HPP
#define _SHARPEN_FILEOPS_HPP

#include "FileTypeDef.hpp"
#include "TypeDef.hpp"
#include "SystemMacro.hpp"

namespace sharpen
{
    bool AccessFile(const char *name,sharpen::FileAccessModel model);

    bool ExistFile(const char *name);

    void RenameFile(const char *oldName,const char *newName);

    void RemoveFile(const char *name);

    bool GetCurrentWorkDirectory(char *pathBuf,sharpen::Size size) noexcept;

    void SetCurrentWorkDirectory(const char *path);

    constexpr inline sharpen::Size GetMaxPath() noexcept
    {
#ifdef SHARPEN_IS_WIN
        return 260; // MAX_PATH
#else
        return 4096; // PATH_MAX
#endif
    }

    void ResolvePath(const char *currentPath,sharpen::Size currentPathSize,const char *path,sharpen::Size pathSize,char *resolvedPath,sharpen::Size resolvedPathSize);

    constexpr inline bool IsPathSeparator(char c) noexcept
    {
#ifdef SHARPEN_IS_WIN
        return c == '\\' || c == '/';
#else
        return c == '/';
#endif
    }

    void MakeDirectory(const char *name);

    void DeleteDirectory(const char *name);
}

#endif