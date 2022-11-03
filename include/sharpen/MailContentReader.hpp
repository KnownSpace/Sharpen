#pragma once
#ifndef _SHARPEN_MAILCONTENTREADER_HPP
#define _SHARPEN_MAILCONTENTREADER_HPP

#include <cassert>

#include "IMailContent.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class MailContentReader:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::MailContentReader;
    
        std::size_t offset_;
        const sharpen::IMailContent *content_;
    public:
    
        explicit MailContentReader(const sharpen::IMailContent *content)
            :offset_(0)
            ,content_(content)
        {
            assert(this->content_);
        }
    
        MailContentReader(Self &&other) noexcept
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
    
        ~MailContentReader() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        template<typename _T>
        inline void Read(sharpen::BinarySerializable<_T> &obj)
        {
            assert(this->content_);
            this->offset_ += this->content_->Unserialize(obj,this->offset_);
        }

        inline bool Readable() const noexcept
        {
            return this->content_;
        }

        inline std::size_t GetOffset() const noexcept
        {
            return this->offset_;
        }

        inline const sharpen::IMailContent &Content() const noexcept
        {
            assert(this->content_);
            return *this->content_;
        }
    };
}

#endif