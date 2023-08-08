#pragma once
#ifndef _SHARPEN_FILETYPEDEF_HPP
#define _SHARPEN_FILETYPEDEF_HPP

#include "SystemMacro.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

#ifdef SHARPEN_IS_WIN
struct _stat64;
#else
struct stat64;
#endif

namespace sharpen {

#ifdef SHARPEN_IS_WIN
    using FileHandle = void *;
    using IoSizeType = std::int32_t;
    using FileStat = struct ::_stat64;
#else
    using FileHandle = int;
    using IoSizeType = std::int64_t;
    using FileStat = struct ::stat64;
#endif

    
    constexpr std::size_t MaxIoSize = static_cast<std::size_t>((std::numeric_limits<IoSizeType>::max)());

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
