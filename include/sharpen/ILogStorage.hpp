#pragma once
#ifndef _SHARPEN_ILOGSTORAGE_HPP
#define _SHARPEN_ILOGSTORAGE_HPP

#include "ByteBuffer.hpp"
#include "Optional.hpp"

namespace sharpen
{
    class ILogStorage
    {
    private:
        using Self = sharpen::ILogStorage;
    protected:

        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(std::uint64_t index) const = 0;

        virtual sharpen::Optional<std::uint64_t> NviLookupTerm(std::uint64_t index) const = 0;

        virtual void NviWrite(std::uint64_t index,sharpen::ByteSlice log) = 0;
    public:
    
        constexpr static std::uint64_t noneIndex{0};

        ILogStorage() noexcept = default;
    
        ILogStorage(const Self &other) noexcept = default;
    
        ILogStorage(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~ILogStorage() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual std::uint64_t GetLastIndex() const = 0;

        inline sharpen::Optional<std::uint64_t> LookupTerm(std::uint64_t index) const
        {
            if(index != noneIndex)
            {
                return this->NviLookupTerm(index);
            }
            return sharpen::EmptyOpt;
        }

        inline sharpen::Optional<sharpen::ByteBuffer> Lookup(std::uint64_t index) const
        {
            if(index != noneIndex)
            {
                return this->NviLookup(index);
            }
            return sharpen::EmptyOpt;
        }

        inline void Write(std::uint64_t index,sharpen::ByteSlice log)
        {
            assert(index != noneIndex);
            this->NviWrite(index,log);
        }

        inline void Write(std::uint64_t index,const sharpen::ByteBuffer &log)
        {
            this->Write(index,log.GetSlice());
        }
    };
}

#endif