#pragma once
#ifndef _SHARPEN_ACTORID_HPP
#define _SHARPEN_ACTORID_HPP

#include "CommonId.hpp"

namespace sharpen {
    class ActorId:public sharpen::CommonId {
    private:
        using Self = sharpen::ActorId;

    public:
        ActorId() noexcept = default;

        ActorId(const Self &other) noexcept = default;

        ActorId(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        ~ActorId() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }
    };
}   // namespace sharpen

#endif