#pragma once
#ifndef _SHARPEN_THREADINFO_HPP
#define _SHARPEN_THREADINFO_HPP

#include "SystemMacro.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    extern sharpen::Uint32 GetCurrentThreadId() noexcept;
}

#endif