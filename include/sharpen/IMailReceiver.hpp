#pragma once
#ifndef _SHARPEN_IMAILBOX_HPP
#define _SHARPEN_IMAILBOX_HPP

#include <iterator>

#include "Mail.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    class IMailReceiver
    {
    private:
        using Self = sharpen::IMailReceiver;
    protected:

        virtual void DoReceiveMail(sharpen::Mail mail) = 0;

        template<typename _Iterator,typename _Check = decltype(std::declval<Self&>().DoReceiveMail(*std::declval<_Iterator&>()++))>
        inline void DoReceiveMails(_Iterator begin,_Iterator end,...)
        {
            while (begin != end)
            {
                this->DoReceiveMail(*begin);
                ++begin;
            }
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self&>().DoReceiveMail(*std::declval<_Iterator&>()++))>
        inline void DoReceiveMails(std::move_iterator<_Iterator> begin,std::move_iterator<_Iterator> end,int)
        {
            while (begin != end)
            {
                this->DoReceiveMail(std::move(*begin));
                ++begin;
            }
        }
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

        template<typename _Iterator,typename _Check = decltype(std::declval<Self&>().DoReceiveMails(std::declval<_Iterator&>(),std::declval<_Iterator&>(),0))>
        inline void ReceiveMails(_Iterator begin,_Iterator end)
        {
            this->DoReceiveMails(begin,end,0);
        }

        template<typename ..._Args,typename _Check = decltype(sharpen::Mail{std::declval<_Args>()...})>
        inline void ReceiveMail(_Args &&...args)
        {
            this->DoReceiveMail(sharpen::Mail{std::forward<_Args>(args)...});
        }
    };
}

#endif