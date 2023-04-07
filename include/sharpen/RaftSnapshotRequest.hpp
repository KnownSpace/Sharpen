#pragma once
#ifndef _SHARPEN_RAFTSNAPSHOTREQUEST_HPP
#define _SHARPEN_RAFTSNAPSHOTREQUEST_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#include "ByteBuffer.hpp"
#include "RaftSnapshotMetadata.hpp"
#include "BinarySerializable.hpp"

namespace sharpen
{
    class RaftSnapshotRequest:public sharpen::BinarySerializable<sharpen::RaftSnapshotRequest>
    {
    private:
        using Self = sharpen::RaftSnapshotRequest;

        sharpen::RaftSnapshotMetadata metadata_;
        bool last_;
        sharpen::ByteBuffer data_;
    public:
    
        explicit RaftSnapshotRequest(sharpen::RaftSnapshotMetadata metadata) noexcept;

        RaftSnapshotRequest(sharpen::RaftSnapshotMetadata metadata,sharpen::ByteBuffer data) noexcept;

        RaftSnapshotRequest(sharpen::RaftSnapshotMetadata metadata,sharpen::ByteBuffer data,bool last) noexcept;
    
        RaftSnapshotRequest(const Self &other) = default;
    
        RaftSnapshotRequest(Self &&other) noexcept;
    
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
    
        ~RaftSnapshotRequest() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::RaftSnapshotMetadata &Metadata() noexcept
        {
            return this->metadata_;
        }
        
        inline const sharpen::RaftSnapshotMetadata &Metadata() const noexcept
        {
            return this->metadata_;
        }

        inline sharpen::ByteBuffer &Data() noexcept
        {
            return this->data_;
        }
        
        inline const sharpen::ByteBuffer &Data() const noexcept
        {
            return this->data_;
        }

        inline bool IsLast() const noexcept
        {
            return this->last_;
        }

        inline void SetLast(bool last) noexcept
        {
            this->last_ = last;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t LoadFrom(const char *data,std::size_t size);
    };
}

#endif