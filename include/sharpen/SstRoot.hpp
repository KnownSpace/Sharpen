#pragma once
#ifndef _SHARPEN_SSTROOT_HPP
#define _SHARPEN_SSTROOT_HPP
/*
Sorted String Table
+------------------+
|Data Blocks       |
+------------------+
|Filter Blocks     |
+------------------+
|Meta Index Block  | 
+------------------+
|Meta Index Chksum | 2 bytes - Crc16
+------------------+
|Index Block       | 
+------------------+
|Index Chksum      | 2 bytes - Crc16
+------------------+
|Footer Block      | 32 bytes
+------------------+

Footer Block
+----------------------------+
|Offset of Index Blocks      |  8 bytes
+----------------------------+
|Size of Index Blocks        |  8 bytes
+----------------------------+
|Offset of Meta Index Blocks |  8 bytes
+----------------------------+
|Size of Meta Index Blocks   |  8 bytes
+----------------------------+

Index Block
+---------------------+
|Data Block1 Key Size | 8 bytes
+---------------------+
|Data Block1 Key      |
+---------------------+
|Offset of Data Block1| 8 bytes
+---------------------+
|Size of Data Block1  | 8 bytes
+---------------------+
|        .....        |
+---------------------+
|Data BlockN Key Size | 8 bytes
+---------------------+
|Data BlockN Key      |
+---------------------+
|Offset of Data BlockN| 8 bytes
+---------------------+
|Size of Data BlockN  | 8 bytes
+---------------------+

Meta Index Block
+------------------------+
|Filter Block1 Key size  | 8 bytes
+------------------------+
|Filter Block1 Key       |
+------------------------+
|Offset of Filter Block1 | 8 bytes
+------------------------+
|Size of Filter Block1   | 8 bytes
+------------------------+
|         .....          |
+------------------------+
|Filter BlockN Key size  |
+------------------------+
|Filter BlockN Key       |
+------------------------+
|Offset of Filter BlockN | 8 bytes
+------------------------+
|Size of Filter BlockN   | 8 bytes
+------------------------+

Filter Block
+-------------------+
|defined by user    |
+-------------------+

Data Block
+-------------------+
|defined by user    |
+-------------------+
*/

#include <vector>

#include <cstdint>
#include <cstddef>
#include "SstFooter.hpp"
#include "SstIndexBlock.hpp"
#include "DataCorruptionException.hpp"

namespace sharpen
{
    class SstRoot
    {        
    private:
        using Self = sharpen::SstRoot;

        //load from file
        mutable sharpen::SstFooter footer_;
        sharpen::SstIndexBlock indexBlock_;
        sharpen::SstIndexBlock metaIndexBlock_;
    public:
        SstRoot() = default;
    
        SstRoot(const Self &other) = default;
    
        SstRoot(Self &&other) noexcept = default;
    
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
                this->footer_ = std::move(other.footer_);
                this->indexBlock_ = std::move(other.indexBlock_);
                this->metaIndexBlock_ = std::move(other.metaIndexBlock_);
            }
            return *this;
        }
    
        ~SstRoot() noexcept = default;

        sharpen::SstFooter &Footer() noexcept
        {
            return this->footer_;
        }

        const sharpen::SstFooter &Footer() const noexcept
        {
            return this->footer_;
        }

        sharpen::SstIndexBlock &IndexBlock() noexcept
        {
            return this->indexBlock_;
        }

        const sharpen::SstIndexBlock &IndexBlock() const noexcept
        {
            return this->indexBlock_;
        }

        sharpen::SstIndexBlock &MetaIndexBlock() noexcept
        {
            return this->metaIndexBlock_;
        }

        const sharpen::SstIndexBlock &MetaIndexBlock() const noexcept
        {
            return this->metaIndexBlock_;
        }

        void LoadFrom(sharpen::FileChannelPtr channel);

        void StoreTo(sharpen::FileChannelPtr channel,std::uint64_t offset) const;
    };
}

#endif