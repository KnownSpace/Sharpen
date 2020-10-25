#pragma once
#ifndef _SHARPEN_NONMOVABLE_HPP

namespace sharpen
{
    class Nonmovable
    {
        using Self = sharpen::Nonmovable;
    public:
        Nonmovable() = default;

        Nonmovable(const Self &) = default;

        Nonmovable(Self &&) noexcept = delete;

        ~Nonmovable() noexcept = default;
    };
    
}

#endif
