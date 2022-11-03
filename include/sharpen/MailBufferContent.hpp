#pragma once
#ifndef _SHARPEN_MAILBUFFERCONTENT_HPP
#define _SHARPEN_MAILBUFFERCONTENT_HPP

#include "IMailContent.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class MailBufferContent:public sharpen::IMailContent
    {
    private:
        using Self = sharpen::MailBufferContent;
    
        sharpen::ByteBuffer content_;

        virtual const char *DoContent() const noexcept override;

        virtual char *DoContent() noexcept override;

        virtual std::size_t DoGetSize() const noexcept override;

        virtual void DoResize(std::size_t newSize) override;
    public:
    
        MailBufferContent() = default;

        explicit MailBufferContent(sharpen::ByteBuffer content);
    
        MailBufferContent(const Self &other) = default;
    
        MailBufferContent(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->content_ = std::move(other.content_);
            }
            return *this;
        }
    
        virtual ~MailBufferContent() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline virtual bool Extensible() const noexcept override
        {
            return true;
        }
    };
}

#endif