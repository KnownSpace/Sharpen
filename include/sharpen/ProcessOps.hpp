#pragma once
#ifndef _PROCESSOPS_PROCESSOPS_HPP
#define _PROCESSOPS_PROCESSOPS_HPP

#include <cstddef>
#include <cstdint>

namespace sharpen {
    void SuspendProcess(std::uint32_t processId);

    void ResumeProcess(std::uint32_t processId);
}   // namespace sharpen

#endif