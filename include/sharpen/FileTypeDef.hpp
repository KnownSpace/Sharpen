#pragma once
#ifndef _SHARPEN_FILETYPEDEF_HPP
#define _SHARPEN_FILETYPEDEF_HPP

#include "SystemMacro.hpp"

namespace sharpen
{
#ifdef SHARPEN_IS_WIN
    using FileHandle = void*;
#else
    using FileHandle = int;
#endif

    enum class FileOpenMethod
    {
        Open,
        CreateNew,
        CreateOrOpen
    };

    enum class FileAccessMethod
    {
        Read,
        Write,
        All
    };

    enum class FileIoMethod
    {
        Normal,
        Direct,
        Sync,
        DirectAndSync
    };
}

#endif
