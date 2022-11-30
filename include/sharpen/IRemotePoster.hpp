#pragma once
#ifndef _SHARPEN_IREMOTEACTOR_HPP
#define _SHARPEN_IREMOTEACTOR_HPP

#include <memory>

#include "Mail.hpp"
#include "Future.hpp"

namespace sharpen
{
    class IRemotePoster
    {
    private:
        using Self = sharpen::IRemotePoster;
    protected:

        virtual std::uint64_t DoGetId() const noexcept = 0;

        virtual sharpen::Mail DoPost(const sharpen::Mail &mail) = 0;

        virtual void DoClose() noexcept = 0;

        virtual void DoOpen() = 0;
    public:
    
        IRemotePoster() noexcept = default;
    
        IRemotePoster(const Self &other) noexcept = default;
    
        IRemotePoster(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRemotePoster() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void Open() 
        {
            return this->DoOpen();
        }

        inline void Close() noexcept
        {
            return this->DoClose();
        }

        inline sharpen::Mail Post(const sharpen::Mail &mail)
        {
            return this->DoPost(mail);
        }

        inline std::uint64_t GetId() const noexcept
        {
            return this->DoGetId();
        }
    };
}

#endif