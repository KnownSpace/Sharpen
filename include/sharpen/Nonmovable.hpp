#pragma once
#ifndef _SHARPEN_NONMOVABLE_HPP

namespace sharpen {
    class Nonmovable {
        using Self = sharpen::Nonmovable;

    public:
        Nonmovable() noexcept = default;

        Nonmovable(const Self &) noexcept = default;

        Nonmovable(Self &&) noexcept = delete;

        Self &operator=(const Self &) noexcept = default;

        Self &operator=(Self &&other) noexcept = delete;

        ~Nonmovable() noexcept = default;
    };

}   // namespace sharpen

#endif
