#pragma once
#ifndef _SHARPEN_FILETYPEDEF_HPP
#define _SHARPEN_FILETYPEDEF_HPP

#include "SystemMacro.hpp"
#include <cstddef>

namespace sharpen {
#ifdef SHARPEN_IS_WIN
    using FileHandle = void *;
#else
    using FileHandle = int;
#endif

    enum class FileOpenMethod {
        Open,
        CreateNew,
        CreateOrOpen
    };

    enum class FileAccessMethod {
        Read,
        Write,
        All
    };

    enum class FileIoMethod {
        Normal,
        Direct,
        Sync,
        DirectAndSync
    };

    enum class MkdirResult {
        CreateNew,
        Exists
    };

    enum class RmdirResult {
        Removed,
        NotExists
    };

    
    enum class FileEntryType {
        File,
        Directory,
        Pipe,
        UnixSocket,
        SymbolicLink,
        BlockDevice,
        CharDevice
    };

    constexpr inline std::size_t GetMaxPath() noexcept {
#ifdef SHARPEN_IS_WIN
        return 260;   // MAX_PATH
#else
        return 4096;   // PATH_MAX
#endif
    }
}   // namespace sharpen

#endif
