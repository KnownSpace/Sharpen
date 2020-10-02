#pragma once
#ifndef _SHARPEN_NONCOPYABLE_HPP
#define _SHARPEN_NONCOPYABLE_HPP

namespace sharpen
{
    class Noncopyable
    {
        using Self = sharpen::Noncopyable;
    public:
        Noncopyable() = default;

        Noncopyable(const Self &) = delete;

        Noncopyable(Self &&) noexcept = default;

        virtual ~Noncopyable() noexcept = default;
    };
} 

#endif