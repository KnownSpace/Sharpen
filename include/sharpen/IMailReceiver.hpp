#pragma once
#ifndef _SHARPEN_IMAILBOX_HPP
#define _SHARPEN_IMAILBOX_HPP

#include <iterator>
#include <cassert>

#include "Mail.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    class IMailReceiver
    {
    private:
        using Self = sharpen::IMailReceiver;
    protected:

        virtual void DoReceiveMail(sharpen::Mail mail,std::uint64_t actorId) = 0;
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

        inline void ReceiveMail(sharpen::Mail mail,std::uint64_t actorId)
        {
            assert(!mail.Header().Empty() || !mail.Content().Empty());
            this->DoReceiveMail(std::move(mail),actorId);
        }
    };
}

#endif