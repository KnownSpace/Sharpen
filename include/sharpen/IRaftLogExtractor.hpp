#pragma once
#ifndef _SHARPEN_IRAFTLOGEXTRACTOR_HPP
#define _SHARPEN_IRAFTLOGEXTRACTOR_HPP

#include <cstdint>
#include <cstddef>

#include "CorruptedDataError.hpp"
#include "ByteSlice.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IRaftLogExtractor
    {
    private:
        using Self = sharpen::IRaftLogExtractor;
    protected:

        virtual std::uint64_t NviExtractTerm(sharpen::ByteSlice log) const = 0;
    public:
    
        IRaftLogExtractor() noexcept = default;
    
        IRaftLogExtractor(const Self &other) noexcept = default;
    
        IRaftLogExtractor(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftLogExtractor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t ExtractTerm(sharpen::ByteSlice log) const
        {
            if(!log.Empty())
            {
                return this->NviExtractTerm(log);
            }
            throw sharpen::CorruptedDataError{"corrupted raft log"};
        }

        inline std::uint64_t ExtractTerm(const sharpen::ByteBuffer &log) const
        {
            if(!log.Empty())
            {
                return this->NviExtractTerm(log.GetSlice());
            }
            throw sharpen::CorruptedDataError{"corrupted raft log"};
        }
    };
}

#endif