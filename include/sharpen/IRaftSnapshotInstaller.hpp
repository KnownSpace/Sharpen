#pragma once
#ifndef _SHARPEN_IRAFTSNAPSHOTINSTALLER_HPP
#define _SHARPEN_IRAFTSNAPSHOTINSTALLER_HPP

#include <cassert>

#include "RaftSnapshotMetadata.hpp"
#include "Optional.hpp"
#include "ByteSlice.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class IRaftSnapshotInstaller
    {
    private:
        using Self = sharpen::IRaftSnapshotInstaller;
    protected:
    
        virtual void NviWrite(std::uint64_t offset,sharpen::ByteSlice snapshotChunk) = 0;

        virtual void NviInstall(sharpen::RaftSnapshotMetadata metadata) = 0;

        virtual void NviReset() = 0;
    public:
    
        IRaftSnapshotInstaller() noexcept = default;
    
        IRaftSnapshotInstaller(const Self &other) noexcept = default;
    
        IRaftSnapshotInstaller(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftSnapshotInstaller() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::Optional<sharpen::RaftSnapshotMetadata> GetLastMetadata() const = 0;
    
        inline void Write(std::uint64_t offset,sharpen::ByteSlice snapshotChunk)
        {
            if(!snapshotChunk.Empty())
            {
                return this->NviWrite(offset,snapshotChunk);
            }
        }

        inline void Write(std::uint64_t offset,const sharpen::ByteBuffer &snapshotChunk)
        {
            return this->Write(offset,snapshotChunk.GetSlice());
        }

        inline void Install(sharpen::RaftSnapshotMetadata metadata)
        {
            assert(metadata.GetLastIndex() != 0);
            assert(metadata.GetLastTerm() != 0);
            this->NviInstall(metadata);
        }

        inline void Reset()
        {
            this->NviReset();
        }

        virtual std::uint64_t GetExpectedOffset() const noexcept = 0;
    };    
}

#endif