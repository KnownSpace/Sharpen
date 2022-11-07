#pragma once
#ifndef _SHARPEN_FILEOPS_HPP
#define _SHARPEN_FILEOPS_HPP

#include "FileTypeDef.hpp"
#include <cstdint>
#include <cstddef>
#include "SystemMacro.hpp"

namespace sharpen
{
    bool AccessFile(const char *name,sharpen::FileAccessMethod model);

    bool ExistFile(const char *name);

    void RenameFile(const char *oldName,const char *newName);

    void RemoveFile(const char *name);

    bool GetCurrentWorkDirectory(char *pathBuf,std::size_t size) noexcept;

    void SetCurrentWorkDirectory(const char *path);

    constexpr inline std::size_t GetMaxPath() noexcept
    {
#ifdef SHARPEN_IS_WIN
        return 260; // MAX_PATH
#else
        return 4096; // PATH_MAX
#endif
    }

    void ResolvePath(const char *currentPath,std::size_t currentPathSize,const char *path,std::size_t pathSize,char *resolvedPath,std::size_t resolvedPathSize);

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