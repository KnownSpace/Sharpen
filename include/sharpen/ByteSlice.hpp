#pragma once
#ifndef _SHARPEN_BYTESLICE_HPP
#define _SHARPEN_BYTESLICE_HPP

#include <utility>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace sharpen
{
    class ByteSlice
    {
    private:
        using Self = sharpen::ByteSlice;
    
        const char *data_;
        std::size_t size_;
    public:
    
        ByteSlice()
            :data_(nullptr)
            ,size_(0)
        {}

        ByteSlice(const char *data,std::size_t size)
            :data_(data)
            ,size_(size)
        {}
    
        ByteSlice(const Self &other) = default;
    
        ByteSlice(Self &&other) noexcept
            :data_(other.data_)
            ,size_(other.size_)
        {
            other.data_ = nullptr;
            other.size_ = 0;
        }
    
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
                this->data_ = other.data_;
                this->size_ = other.size_;
                other.data_ = nullptr;
                other.size_ = 0;
            }
            return *this;
        }
    
        ~ByteSlice() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline char Get(std::size_t index) const
        {
            if(index > this->size_)
            {
                throw std::out_of_range{"index out of range"};
            }
            return this->data_[index];
        }

        inline const char *Data() const noexcept
        {
            return this->data_;
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->size_;
        }

        inline bool Empty() const noexcept
        {
            return !this->size_;
        }

        inline operator bool() const noexcept
        {
            return this->size_;
        }

        inline char operator[](std::size_t index) const
        {
            return this->Get(index);
        }
    };   
}

#endif