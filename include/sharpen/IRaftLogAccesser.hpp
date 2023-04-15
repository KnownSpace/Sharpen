#pragma once
#ifndef _SHARPEN_IRAFTLOGACCESSER_HPP
#define _SHARPEN_IRAFTLOGACCESSER_HPP

#include "ByteBuffer.hpp"

namespace sharpen
{
    class IRaftLogAccesser
    {
    private:
        using Self = IRaftLogAccesser;
    protected:

        virtual std::uint64_t NviGetTerm(sharpen::ByteSlice logEntry) const noexcept = 0;

        virtual void NviSetTerm(sharpen::ByteBuffer &logEntry,std::uint64_t term) const = 0;

        virtual bool NviIsRaftEntry(sharpen::ByteSlice logEntry) const noexcept = 0;

        virtual sharpen::ByteBuffer NviCreateEntry(sharpen::ByteSlice bytes,std::uint64_t term) const = 0;
    public:

        IRaftLogAccesser() noexcept = default;

        IRaftLogAccesser(const Self &other) noexcept = default;

        IRaftLogAccesser(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IRaftLogAccesser() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline bool IsRaftEntry(sharpen::ByteSlice logEntry) const noexcept
        {
            if(!logEntry.Empty())
            {
                return this->NviIsRaftEntry(logEntry);
            }
            return false;
        }

        inline bool IsRaftEntry(const sharpen::ByteBuffer &logEntry) const noexcept
        {
            return this->IsRaftEntry(logEntry.GetSlice());
        }

        inline std::uint64_t GetTerm(sharpen::ByteSlice logEntry) const noexcept
        {
            assert(this->IsRaftEntry(logEntry));
            return this->NviGetTerm(logEntry);
        }

        inline std::uint64_t GetTerm(const sharpen::ByteBuffer logEntry) const noexcept
        {
            return this->GetTerm(logEntry.GetSlice());
        }

        inline sharpen::Optional<std::uint64_t> LookupTerm(sharpen::ByteSlice logEntry) const noexcept
        {
            if(this->IsRaftEntry(logEntry))
            {
                return this->GetTerm(logEntry);
            }
            return sharpen::EmptyOpt;
        }

        inline sharpen::Optional<std::uint64_t> LookupTerm(const sharpen::ByteBuffer &logEntry) const noexcept
        {
            if(this->IsRaftEntry(logEntry))
            {
                return this->GetTerm(logEntry);
            }
            return sharpen::EmptyOpt;
        }

        inline void SetTerm(sharpen::ByteBuffer &logEntry,std::uint64_t term) const
        {
            assert(this->IsRaftEntry(logEntry));
            this->NviSetTerm(logEntry,term);
        }

        inline sharpen::ByteBuffer CreateEntry(sharpen::ByteSlice bytes,std::uint64_t term) const
        {
            return this->NviCreateEntry(bytes,term);
        }

        inline sharpen::ByteBuffer CreateEntry(const sharpen::ByteBuffer &bytes,std::uint64_t term) const
        {
            return this->CreateEntry(bytes.GetSlice(),term);
        }
    };
}

#endif