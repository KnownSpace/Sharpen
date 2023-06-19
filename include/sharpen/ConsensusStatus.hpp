#pragma once
#ifndef _SHARPEN_CONSENSUSSTATUS_HPP
#define _SHARPEN_CONSENSUSSTATUS_HPP

#include <cstdint>

namespace sharpen {
    enum class ConsensusStatus : std::uint32_t {
        Commited,
        Lost
    };
}   // namespace sharpen

#endif