#pragma once
#ifndef _SHARPEN_MAILSLICECONTENT_HPP
#define _SHARPEN_MAILSLICECONTENT_HPP

#include "IMailContent.hpp"
#include "SliceResizeError.hpp"

namespace sharpen
{
    class MailSliceContent:public sharpen::IMailContent
    {
    private:
        using Self = sharpen::MailSliceContent;
    
        char *slice_;
        std::size_t sliceSize_;

        virtual const char *DoContent() const noexcept override;

        virtual char *DoContent() noexcept override;

        virtual std::size_t DoGetSize() const noexcept override;

        virtual void DoResize(std::size_t newSize) override;
    public:
    
        explicit MailSliceContent(char *slice,std::size_t size) noexcept;
    
        MailSliceContent(const Self &other) noexcept = default;
    
        MailSliceContent(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other) noexcept
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        virtual ~MailSliceContent() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif