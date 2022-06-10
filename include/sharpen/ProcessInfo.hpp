#pragma once
#ifndef _SHARPEN_PROCESSINFO_HPP
#define _SHARPEN_PROCESSINFO_HPP

#include <cstdint>
#include <cstddef>
#include "SystemMacro.hpp"

namespace sharpen
{
    extern std::uint32_t GetProcessId() noexcept;
}

#endif