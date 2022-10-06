#pragma once
#ifndef _SHARPEN_HEAPPTR_HPP
#define _SHARPEN_HEAPPTR_HPP

#include <utility>

namespace sharpen
{
    template<typename _T>
    class HeapPtr
    {
    private:
        using Self = sharpen::HeapPtr<_T>;
    
        _T ptr_;
    public:
    
        HeapPtr() noexcept
            :ptr_(nullptr)
        {}

        //implict
        HeapPtr(_T *p) noexcept
            :ptr_(p)
        {}

        HeapPtr(const Self &other) noexcept = default;
    
        HeapPtr(Self &&other) noexcept = default;
    
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
                this->ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        inline Self &operator=(_T *p) noexcept
        {
            this->ptr_ = p;
        }
    
        ~HeapPtr() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline _T *GetPtr() const noexcept
        {
            return this->ptr_;
        }

        operator _T*() const noexcept
        {
            return this->GetPtr();
        }
    };   
}

#endif