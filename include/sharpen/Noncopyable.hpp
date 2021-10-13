#pragma once
#ifndef _SHARPEN_NONCOPYABLE_HPP
#define _SHARPEN_NONCOPYABLE_HPP

namespace sharpen
{
    class Noncopyable
    {
    private:
        using Self = sharpen::Noncopyable;
    public:
        Noncopyable() noexcept = default;

        Noncopyable(const Self &) noexcept = delete;

        Noncopyable(Self &&) noexcept = default;

        Self &operator=(const Self &) noexcept = delete;

        Self &operator=(Self &&other) noexcept = default;

        ~Noncopyable() noexcept = default;
    };
} 

#endif
