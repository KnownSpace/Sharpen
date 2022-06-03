#pragma once
#ifndef _SHARPEN_REVERSEPOINTERITERATOR_HPP
#define _SHARPEN_REVERSEPOINTERITERATOR_HPP

#include <utility>
#include <iterator>

#include "TypeDef.hpp"
#include "IteratorTemplate.hpp"

namespace sharpen
{
    template<typename _T>
    class ReversePointerIterator:public sharpen::IteratorTemplate<std::random_access_iterator_tag,_T,std::ptrdiff_t,_T*,_T&>
    {
    private:
        using Self = ReversePointerIterator;
    
        _T *ptr_;
    public:
    
        explicit ReversePointerIterator(_T *ptr)
            :ptr_(ptr)
        {}
    
        ReversePointerIterator(const Self &other) = default;
    
        ReversePointerIterator(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~ReversePointerIterator() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline _T &operator*() noexcept
        {
            return *this->ptr_;
        }

        inline const _T &operator*() const noexcept
        {
            return *this->ptr_;
        }

        inline _T &operator->() noexcept
        {
            return *this->ptr_;
        }

        inline const _T &operator->() const noexcept
        {
            return *this->ptr_;
        }

        inline Self &operator++() noexcept
        {
            this->ptr_ -= 1;
            return *this;
        }

        inline Self operator++(int) noexcept
        {
            Self tmp{*this};
            this->ptr_ -= 1;
            return tmp;
        }

        inline Self &operator--() noexcept
        {
            this->ptr_ += 1;
            return *this;
        }

        inline Self operator--(int) noexcept
        {
            Self tmp{*this};
            this->ptr_ += 1;
            return tmp;
        }

        inline Self &operator+=(std::ptrdiff_t diff) noexcept
        {
            this->ptr_ -= diff;
            return *this;
        }

        inline Self &operator-=(std::ptrdiff_t diff) noexcept
        {
            this->ptr_ += diff;
            return *this;
        }

        inline sharpen::Int32 CompareWith(const Self &other) const noexcept
        {
            if(this->ptr_ > other.ptr_)
            {
                return 1;
            }
            else if(this->ptr_ < other.ptr_)
            {
                return -1;
            }
            return 0;
        }
        
        inline bool operator==(const Self &other) const noexcept
        {
            return this->CompareWith(other) == 0;
        }
        
        inline bool operator!=(const Self &other) const noexcept
        {
            return this->CompareWith(other) != 0;
        }
        
        inline bool operator<(const Self &other) const noexcept
        {
            return this->CompareWith(other) < 0;
        }
        
        inline bool operator>(const Self &other) const noexcept
        {
            return this->CompareWith(other) > 0;
        }
        
        inline bool operator>=(const Self &other) const noexcept
        {
            return this->CompareWith(other) >= 0;
        }
        
        inline bool operator<=(const Self &other) const noexcept
        {
            return this->CompareWith(other) <= 0;
        }

        inline _T *GetPointer() const noexcept
        {
            return this->ptr_;
        }

        inline std::ptrdiff_t operator-(const Self &other) const noexcept
        {
            return other.ptr_ - this->ptr_;
        }
    };

    template<typename _T>
    inline sharpen::ReversePointerIterator<_T> operator+(sharpen::ReversePointerIterator<_T> ite,std::ptrdiff_t value)
    {
        return ite += value;
    }

    template<typename _T>
    inline sharpen::ReversePointerIterator<_T> operator-(sharpen::ReversePointerIterator<_T> ite,std::ptrdiff_t value)
    {
        return ite -= value;
    }

    template<typename _T>
    inline sharpen::ReversePointerIterator<_T> operator+(std::ptrdiff_t value,sharpen::ReversePointerIterator<_T> ite)
    {
        return ite += value;
    }

    template<typename _T>
    inline sharpen::ReversePointerIterator<_T> operator-(std::ptrdiff_t value,sharpen::ReversePointerIterator<_T> ite)
    {
        return ite -= value;
    }   
}

#endif