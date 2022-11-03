#pragma once
#ifndef _SHARPEN_IMAILCONTENT_HPP
#define _SHARPEN_IMAILCONTENT_HPP

#include "BinarySerializable.hpp"

namespace sharpen
{
    class IMailContent
    {
    private:
        using Self = sharpen::IMailContent;
    protected:

        virtual const char *DoContent() const noexcept = 0;

        virtual char *DoContent() noexcept = 0;

        virtual std::size_t DoGetSize() const noexcept = 0;

        virtual void DoResize(std::size_t newSize) = 0;
    public:

        inline virtual bool Extensible() const noexcept
        {
            return false;
        }
    public:
    
        IMailContent() noexcept = default;
    
        IMailContent(const Self &other) noexcept = default;
    
        IMailContent(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IMailContent() noexcept = default;

        inline char *Content() noexcept
        {
            return this->DoContent();
        }

        inline const char *Content() const noexcept
        {
            return this->DoContent();
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->DoGetSize();
        }

        template<typename _T>
        inline bool Overflow(const sharpen::BinarySerializable<_T> &object) const noexcept
        {
            return this->Overflow(object,0);
        }

        template<typename _T>
        inline bool Overflow(const sharpen::BinarySerializable<_T> &object,std::size_t offset) const noexcept
        {
            return object.ComputeSize() + offset > this->GetSize();
        }

        template<typename _T>
        inline std::size_t Unserialize(sharpen::BinarySerializable<_T> &object) const
        {
            return this->Unserialize(object,0);
        }

        template<typename _T>
        inline std::size_t Unserialize(sharpen::BinarySerializable<_T> &object,std::size_t offset) const
        {
            assert(this->GetSize() > offset);
            return object.LoadFrom(this->Content() + offset,this->GetSize() - offset);
        }

        template<typename _T>
        inline std::size_t Serialize(const sharpen::BinarySerializable<_T> &object)
        {
            return this->Serialize(object,0);
        }

        template<typename _T>
        inline std::size_t Serialize(const sharpen::BinarySerializable<_T> &object,std::size_t offset)
        {
            if(this->Overflow(object,offset))
            {
                std::size_t newSize = object.ComputeSize() + offset;
                this->DoResize(newSize);
            }
            return object.UnsafeStoreTo(this->Content() + offset);
        }
    };
}

#endif