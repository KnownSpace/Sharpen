#pragma once
#ifndef _SHARPEN_SSTVECTOR_HPP
#define _SHARPEN_SSTVECTOR_HPP

#include <cassert>

#include "SortedStringTable.hpp"
#include "IFileChannel.hpp"

namespace sharpen
{
    class SstVector
    {
    private:
        using Self = sharpen::SstVector;
    
        const sharpen::SortedStringTable *root_;
        sharpen::FileChannelPtr channel_;
    public:
    
        SstVector() noexcept
            :root_(nullptr)
            ,channel_(nullptr)
        {}

        SstVector(const sharpen::SortedStringTable *root,sharpen::FileChannelPtr channel) noexcept
            :root_(root)
            ,channel_(std::move(channel))
        {
            assert(this->root_);
        }
    
        SstVector(const Self &other) = default;
    
        SstVector(Self &&other) noexcept
            :root_(other.root_)
            ,channel_(std::move(other.channel_))
        {
            other.root_ = nullptr;
        }
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->root_ = other.root_;
                this->channel_ = std::move(other.channel_);
                other.root_ = nullptr;
            }
            return *this;
        }
    
        ~SstVector() noexcept = default;

        sharpen::FileChannelPtr &Channel() noexcept
        {
            return this->channel_;
        }

        const sharpen::FileChannelPtr &Channel() const noexcept
        {
            return this->channel_;
        }

        const sharpen::SortedStringTable &Root() const noexcept
        {
            assert(this->root_);
            return *this->root_;
        }
    };
}

#endif