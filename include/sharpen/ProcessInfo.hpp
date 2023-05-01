#pragma once
#ifndef _SHARPEN_PROCESSINFO_HPP
#define _SHARPEN_PROCESSINFO_HPP

#include "SystemMacro.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen {
    extern std::uint32_t GetProcessId() noexcept;
}

#endif