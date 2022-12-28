#pragma once
#ifndef _SHARPEN_IREMOTEACTOR_HPP
#define _SHARPEN_IREMOTEACTOR_HPP

#include <memory>

#include "Mail.hpp"
#include "Future.hpp"
#include "IMailParser.hpp"

namespace sharpen
{
    class IRemotePoster
    {
    private:
        using Self = sharpen::IRemotePoster;
    protected:

        virtual std::uint64_t NviGetId() const noexcept = 0;

        virtual sharpen::Mail NviPost(const sharpen::Mail &mail) = 0;

        virtual void NviClose() noexcept = 0;

        virtual void NviOpen(std::unique_ptr<sharpen::IMailParser> parser) = 0;
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

        inline void Open(std::unique_ptr<sharpen::IMailParser> parser) 
        {
            assert(parser != nullptr);
            return this->NviOpen(std::move(parser));
        }

        inline void Close() noexcept
        {
            return this->NviClose();
        }

        inline sharpen::Mail Post(const sharpen::Mail &mail)
        {
            return this->NviPost(mail);
        }

        inline std::uint64_t GetId() const noexcept
        {
            return this->NviGetId();
        }
    };
}

#endif