#pragma once
#ifndef _SHARPEN_BARRIERMODEL_HPP
#define _SHARPEN_BARRIERMODEL_HPP

namespace sharpen {
    // control the behavior of reset
    enum class BarrierModel {
        // set count to 0
        Flush,
        // count -= bound
        Boundaried
    };
}   // namespace sharpen

#endif