#pragma once
#ifndef _SHARPEN_IRAFTSNAPSHOT_HPP
#define _SHARPEN_IRAFTSNAPSHOT_HPP

#include "IRaftSnapshotChunk.hpp"

namespace sharpen
{
    class IRaftSnapshot
    {
    private:
        using Self = sharpen::IRaftSnapshot;
    protected:
    public:
    
        IRaftSnapshot() noexcept = default;
    
        IRaftSnapshot(const Self &other) noexcept = default;
    
        IRaftSnapshot(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftSnapshot() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual std::uint64_t GetLastIndex() const noexcept = 0;

        virtual std::uint64_t GetLastTerm() const noexcept = 0;

        virtual std::unique_ptr<sharpen::IRaftSnapshotChunk> GetChainedChunks() const = 0;
    };
}

#endif