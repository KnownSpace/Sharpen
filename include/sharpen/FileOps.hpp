#pragma once
#ifndef _SHARPEN_FILEOPS_HPP
#define _SHARPEN_FILEOPS_HPP

#include "FileTypeDef.hpp"
#include "SystemMacro.hpp"
#include "ParameterMacro.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen {
    extern bool AccessFile(const char *name, sharpen::FileAccessMethod model);

    inline bool AccessDirectory(const char *name, sharpen::FileAccessMethod model) {
        return sharpen::AccessFile(name, model);
    }

    extern bool ExistFile(const char *name);

    inline bool ExistDirectory(const char *name) {
        return sharpen::ExistFile(name);
    }

    extern void RenameFile(const char *oldName, const char *newName);

    inline void RenameDirectory(const char *oldName, const char *newName) {
        return sharpen::RenameFile(oldName, newName);
    }

    extern void RemoveFile(const char *name);

    extern bool GetCurrentWorkDirectory(SHARPEN_OUT char *pathBuf, std::size_t size) noexcept;

    extern void SetCurrentWorkDirectory(const char *path);

    extern void ResolvePath(const char *currentPath,
                            std::size_t currentPathSize,
                            const char *path,
                            std::size_t pathSize,
                            SHARPEN_OUT char *resolvedPath,
                            std::size_t resolvedPathSize);

    constexpr inline bool IsPathSeparator(char c) noexcept {
#ifdef SHARPEN_IS_WIN
        return c == '\\' || c == '/';
#else
        return c == '/';
#endif
    }

    extern sharpen::MkdirResult MakeDirectory(const char *name);

    extern sharpen::RmdirResult DeleteDirectory(const char *name);

    extern void MakeLink(const char *oldName, const char *newName);

    extern void MakeSymLink(const char *oldName, const char *newName, bool isDir);

    inline void MakeSymLink(const char *oldName, const char *newName) {
        return sharpen::MakeSymLink(oldName, newName, false);
    }

    inline void MakeDirectorySymLink(const char *oldName, const char *newName) {
        return sharpen::MakeSymLink(oldName, newName, true);
    }

    void GetFileStat(const char *name,SHARPEN_OUT sharpen::FileStat &stat);
}   // namespace sharpen

#endif