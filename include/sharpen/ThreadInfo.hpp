#pragma once
#ifndef _SHARPEN_THREADINFO_HPP
#define _SHARPEN_THREADINFO_HPP

#include "SystemMacro.hpp"
#include <cstdint>
#include <cstddef>

namespace sharpen
{
    extern std::uint32_t GetCurrentThreadId() noexcept;
}

#endif