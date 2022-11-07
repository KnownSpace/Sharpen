#pragma once
#ifndef _SHARPEN_IREMOTEACTOR_HPP
#define _SHARPEN_IREMOTEACTOR_HPP

#include <memory>

#include "IMail.hpp"
#include "Future.hpp"

namespace sharpen
{
    class IRemoteActor
    {
    private:
        using Self = sharpen::IRemoteActor;
    protected:

        virtual std::uint64_t DoGetAddressHash() const noexcept = 0;

        virtual std::unique_ptr<sharpen::IMail> DoPost(const sharpen::IMail &mail) = 0;

        virtual void DoClose() noexcept = 0;

        virtual void DoOpen() = 0;
    public:
    
        IRemoteActor() noexcept = default;
    
        IRemoteActor(const Self &other) noexcept = default;
    
        IRemoteActor(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRemoteActor() noexcept = default;
    
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

        inline std::unique_ptr<sharpen::IMail> Post(const sharpen::IMail &mail)
        {
            return this->DoPost(mail);
        }
    };
}

#endif