#pragma once
#ifndef _SHARPEN_IMAILBOX_HPP
#define _SHARPEN_IMAILBOX_HPP

#include "Mail.hpp"
#include "TypeTraits.hpp"
#include <cassert>
#include <iterator>

namespace sharpen
{
    class IMailReceiver
    {
    private:
        using Self = sharpen::IMailReceiver;

    protected:
        virtual void NviReceive(sharpen::Mail mail, std::uint64_t actorId) = 0;

    public:
        IMailReceiver() noexcept = default;

        IMailReceiver(const Self &other) noexcept = default;

        IMailReceiver(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IMailReceiver() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void Receive(sharpen::Mail mail, std::uint64_t actorId)
        {
            assert(!mail.Empty());
            return this->NviReceive(std::move(mail), actorId);
        }
    };
}   // namespace sharpen

#endif