#pragma once
#ifndef _SHARPEN_SSTBUILDOPTION_HPP
#define _SHARPEN_SSTBUILDOPTION_HPP

#include <utility>

#include "TypeDef.hpp"

namespace sharpen
{
    class SstBuildOption
    {
    private:
        using Self = sharpen::SstBuildOption;

        // 1% - fake positive rate
        static constexpr sharpen::Size defaultBitsOfElement_{10};

        //default block size is 4kb
        static constexpr sharpen::Size defaultBlockSize_{4*1024};

        //erase elements that already deleted
        bool eraseDeleted_;
        //bloom filter config
        sharpen::Size bitsOfElement_;
        //size of every block
        sharpen::Size blockSize_;
    public:

        explicit SstBuildOption(bool eraseDeleted)
            :SstBuildOption(eraseDeleted,Self::defaultBitsOfElement_)
        {}

        SstBuildOption(bool eraseDeleted,sharpen::Size bitsOfElement)
            :SstBuildOption(eraseDeleted,bitsOfElement,Self::defaultBlockSize_)
        {}

        SstBuildOption(bool eraseDeleted,sharpen::Size bitsOfElement,sharpen::Size blockSize)
            :eraseDeleted_(eraseDeleted)
            ,bitsOfElement_(bitsOfElement)
            ,blockSize_(blockSize)
        {}
    
        SstBuildOption(const Self &other) = default;
    
        SstBuildOption(Self &&other) noexcept = default;
    
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
                this->eraseDeleted_ = other.eraseDeleted_;
                this->bitsOfElement_ = other.bitsOfElement_;
                this->blockSize_ = other.blockSize_;
            }
            return *this;
        }
    
        ~SstBuildOption() noexcept = default;

        inline bool IsEraseDeleted() const noexcept
        {
            return this->eraseDeleted_;
        }

        inline sharpen::Size GetFilterBitsOfElement() const noexcept
        {
            return this->bitsOfElement_;
        }

        inline sharpen::Size GetBlockSize() const noexcept
        {
            return this->blockSize_;
        }
    };
}

#endif