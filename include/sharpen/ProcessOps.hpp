#pragma once
#ifndef _PROCESSOPS_PROCESSOPS_HPP
#define _PROCESSOPS_PROCESSOPS_HPP

#include <cstdint>
#include <cstddef>

#include "SystemMacro.hpp"

namespace sharpen
{
    void SuspendProcess(std::uint32_t processId);

    void ResumeProcess(std::uint32_t processId);
}

#endif