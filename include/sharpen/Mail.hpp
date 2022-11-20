#pragma once
#ifndef _SHARPEN_MAIL_HPP
#define _SHARPEN_MAIL_HPP

#include "ByteBuffer.hpp"

namespace sharpen
{
    class Mail
    {
    private:
        using Self = sharpen::Mail;
    
        sharpen::ByteBuffer header_;
        sharpen::ByteBuffer content_;
    public:
    
        Mail() noexcept = default;

        explicit Mail(sharpen::ByteBuffer content) noexcept
            :header_()
            ,content_(content)
        {}

        Mail(sharpen::ByteBuffer header,sharpen::ByteBuffer content) noexcept
            :header_(std::move(header))
            ,content_(std::move(content))
        {}
    
        Mail(const Self &other) = default;
    
        Mail(Self &&other) noexcept
            :header_(std::move(other.header_))
            ,content_(std::move(other.content_))
        {}
    
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
                this->header_ = std::move(other.header_);
                this->content_ = std::move(other.content_);
            }
            return *this;
        }
    
        ~Mail() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::ByteBuffer &Header() noexcept
        {
            return this->header_;
        }
        
        inline const sharpen::ByteBuffer &Header() const noexcept
        {
            return this->header_;
        }

        inline sharpen::ByteBuffer &Content() noexcept
        {
            return this->content_;
        }
        
        inline const sharpen::ByteBuffer &Content() const noexcept
        {
            return this->content_;
        }
    };
}

#endif