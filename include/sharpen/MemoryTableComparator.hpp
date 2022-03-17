#pragma once
#ifndef _SHARPEN_MEMORYTABLECOMPARATOR_HPP
#define _SHARPEN_MEMORYTABLECOMPARATOR_HPP

#include <utility>

#include "TypeDef.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class MemoryTableComparator
    {
    public:
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = sharpen::MemoryTableComparator;
    
        Comparator comp_;
    public:
        MemoryTableComparator()
            :Self(nullptr)
        {}

        explicit MemoryTableComparator(Comparator comp)
            :comp_(comp)
        {}
    
        MemoryTableComparator(const Self &other) = default;
    
        MemoryTableComparator(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->comp_ = other.comp_;
                other.comp_ = nullptr;
            }
            return *this;
        }
    
        ~MemoryTableComparator() noexcept = default;

        bool operator()(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right)const noexcept
        {
            if(this->comp_)
            {
                return this->comp_(left,right) == -1;
            }
            return left < right;
        }
    };
}

#endif