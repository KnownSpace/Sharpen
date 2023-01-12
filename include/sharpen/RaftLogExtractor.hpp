#pragma once
#ifndef _SHARPEN_RAFTLOGEXTRACTOR_HPP
#define _SHARPEN_RAFTLOGEXTRACTOR_HPP

#include "IRaftLogExtractor.hpp"

namespace sharpen
{
    class RaftLogExtractor:public sharpen::IRaftLogExtractor
    {
    private:
        using Self = sharpen::RaftLogExtractor;
    
        virtual std::uint64_t NviExtractTerm(sharpen::ByteSlice log) const override;
    public:
    
        RaftLogExtractor() noexcept = default;
    
        RaftLogExtractor(const Self &other) noexcept = default;
    
        RaftLogExtractor(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other) noexcept
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~RaftLogExtractor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif