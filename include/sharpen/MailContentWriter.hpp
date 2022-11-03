#pragma once
#ifndef _SHARPEN_MAILCONTENTWRITER_HPP
#define _SHARPEN_MAILCONTENTWRITER_HPP

#include <cassert>

#include "IMailContent.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class MailContentWriter:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::MailContentWriter;
    
        std::size_t offset_;
        sharpen::IMailContent *content_;
    public:
    
        explicit MailContentWriter(sharpen::IMailContent *content)
            :offset_(0)
            ,content_(content)
        {
            assert(this->content_);
        }
    
        MailContentWriter(Self &&other) noexcept
            :offset_(other.offset_)
            ,content_(other.content_)
        {}
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->offset_ = other.offset_;
                this->content_ = other.content_;
                other.offset_ = 0;
                other.content_ = nullptr;
            }
            return *this;
        }
    
        ~MailContentWriter() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        template<typename _T>
        inline void Write(const sharpen::BinarySerializable<_T> &obj)
        {
            assert(this->content_);
            if(this->content_->Overflow(obj,this->offset_) && !this->content_->Extensible())
            {
                throw std::out_of_range{"index out of range"};
            }
            this->offset_ += this->content_->Serialize(obj,this->offset_);
        }

        inline bool Writable() const noexcept
        {
            return this->content_;
        }

        inline std::size_t GetLength() const noexcept
        {
            return this->offset_;
        }

        inline sharpen::IMailContent &Content() noexcept
        {
            assert(this->content_);
            return *this->content_;
        }

        inline const sharpen::IMailContent &Content() const noexcept
        {
            assert(this->content_);
            return *this->content_;
        }
    };
}

#endif