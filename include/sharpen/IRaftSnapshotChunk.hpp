#pragma once
#ifndef _SHARPEN_IRAFTSNAPSHOTCHUNK_HPP
#define _SHARPEN_IRAFTSNAPSHOTCHUNK_HPP

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    class IRaftSnapshotChunk
    {
    private:
        using Self = sharpen::IRaftSnapshotChunk;
    protected:
    public:
    
        IRaftSnapshotChunk() noexcept = default;
    
        IRaftSnapshotChunk(const Self &other) noexcept = default;
    
        IRaftSnapshotChunk(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftSnapshotChunk() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual bool Forward() = 0;

        virtual const char *Data() const noexcept = 0;

        virtual std::size_t GetSize() const noexcept = 0;

        virtual bool Forwardable() const = 0;
    };
}

#endif