#pragma once
#ifndef _SHARPEN_CONSENSUSMEMBERCHANGES_HPP
#define _SHARPEN_CONSENSUSMEMBERCHANGES_HPP

#include <set>

#include "ByteBuffer.hpp"

namespace sharpen
{
    class ConsensusChanges
    {
    private:
        using Self = sharpen::ConsensusChanges;
    
        std::set<sharpen::ByteBuffer> addSet_;
        std::set<sharpen::ByteBuffer> removeSet_;
    public:
    
        ConsensusChanges() = default;
    
        ConsensusChanges(const Self &other) = default;
    
        ConsensusChanges(Self &&other) noexcept = default;
    
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
                this->addSet_ = std::move(other.addSet_);
                this->removeSet_ = std::move(other.removeSet_);
            }
            return *this;
        }
    
        ~ConsensusChanges() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::set<sharpen::ByteBuffer> &AddSet() noexcept
        {
            return this->addSet_;
        }
        
        inline const std::set<sharpen::ByteBuffer> &AddSet() const noexcept
        {
            return this->addSet_;
        }

        inline std::set<sharpen::ByteBuffer> &RemoveSet() noexcept
        {
            return this->removeSet_;
        }
        
        inline const std::set<sharpen::ByteBuffer> &RemoveSet() const noexcept
        {
            return this->removeSet_;
        }
    };   
}

#endif