#pragma once
#ifndef _SHARPEN_BYTEVECTOR_HPP
#define _SHARPEN_BYTEVECTOR_HPP

#include <utility>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include "TypeDef.hpp"
#include "IteratorOps.hpp"
#include "PointerIterator.hpp"
#include "ReversePointerIterator.hpp"

namespace sharpen
{
    struct ByteVectorStruct
    {
        char *data_;
        sharpen::Size cap_;
    };

    union ByteVectorUnion
    {
        sharpen::ByteVectorStruct heap_;
        char stack_[sizeof(heap_)];
    };

    class ByteVector
    {
    private:
        using Self = ByteVector;

        static constexpr sharpen::Size inlineSize_{sizeof(sharpen::ByteVectorUnion)};
        static constexpr sharpen::Size blobSize_{1*1024*1024};
    
        sharpen::Size size_;
        sharpen::ByteVectorUnion rawVector_;

        inline static bool InlineBuffer(sharpen::Size size) noexcept
        {
            return size <= inlineSize_;
        }

        inline bool InlineBuffer() const noexcept
        {
            return this->InlineBuffer(this->size_);
        }

        static sharpen::Size ComputeHeapSize(sharpen::Size size) noexcept;

        void MoveFrom(Self &&other) noexcept;

        bool CheckPointer(const char *p);
    public:
        using Iterator = sharpen::PointerIterator<char>;
        using ConstIterator = sharpen::PointerIterator<const char>;
        using ReverseIterator = sharpen::ReversePointerIterator<char>;
        using ConstReverseIterator = sharpen::ReversePointerIterator<const char>;

        ByteVector() noexcept
            :size_(0)
            ,rawVector_()
        {}

        explicit ByteVector(sharpen::Size size);
    
        ByteVector(const Self &other);
    
        ByteVector(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~ByteVector() noexcept
        {
            this->Clear();
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        char *Data() noexcept;

        const char *Data() const noexcept;

        char &Get(sharpen::Size index);

        char Get(sharpen::Size index) const;

        inline sharpen::Size GetSize() const noexcept
        {
            return this->size_;
        }

        void Clear() noexcept;

        void Resize(sharpen::Size newSize,char defaultVal);

        inline void Resize(sharpen::Size newSize)
        {
            this->Resize(newSize,0);
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<char&>() = *std::declval<_Iterator&>()++)>
        inline void Append(_Iterator begin,_Iterator end)
        {
            sharpen::Size size{sharpen::GetRangeSize(begin,end)};
            sharpen::Size oldSize{this->GetSize()};
            sharpen::Size newSize{oldSize + size};
            this->Resize(newSize);
            char *buf = this->Data();
            while(oldSize != newSize)
            {
                buf[oldSize++] = *begin++;
            }
        }

        inline void PushBack(char c)
        {
            this->Append(&c,&c + 1);
        }

        void Erase(sharpen::Size begin,sharpen::Size end) noexcept;

        inline void Erase(sharpen::Size where) noexcept
        {
            return this->Erase(where,where + 1);
        }

        void Erase(Iterator where);

        void Erase(Iterator begin,Iterator end);

        void Erase(ConstIterator where);

        void Erase(ConstIterator begin,ConstIterator end);

        inline void PopBack()
        {
            if(this->size_)
            {
                this->Erase(this->size_ - 1);
            }
        }

        inline bool Empty() const noexcept
        {
            return !this->size_;
        }

        Iterator Begin() noexcept;
        
        ConstIterator Begin() const noexcept;
        
        Iterator End() noexcept;
        
        ConstIterator End() const noexcept;

        ReverseIterator ReverseBegin() noexcept;
        
        ConstReverseIterator ReverseBegin() const noexcept;
        
        ReverseIterator ReverseEnd() noexcept;
        
        ConstReverseIterator ReverseEnd() const noexcept;

        char &operator[](sharpen::Size index) noexcept
        {
            return this->Data()[index];
        }

        const char operator[](sharpen::Size index) const noexcept
        {
            return this->Data()[index];
        }

        inline char &Front()
        {
            return this->Get(0);
        }

        inline char Front() const noexcept
        {
            return this->Get(0);
        }

        inline char &Back()
        {
            if(this->Empty())
            {
                throw std::out_of_range("index out of range");
            }
            return this->Get(this->GetSize() - 1);
        }

        inline char Back() const
        {
            if(this->Empty())
            {
                throw std::out_of_range("index out of range");
            }
            return this->Get(this->GetSize() - 1);
        }
    };
}   

#endif