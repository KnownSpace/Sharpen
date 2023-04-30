#pragma once
#ifndef _SHARPEN_IRAFTSNAPSHOTCHUNK_HPP
#define _SHARPEN_IRAFTSNAPSHOTCHUNK_HPP

#include "ByteBuffer.hpp"
#include <cstddef>
#include <cstdint>

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

        virtual void Forward() = 0;

        virtual sharpen::ByteBuffer GenerateChunkData() const = 0;

        virtual bool Forwardable() const = 0;

        virtual std::uint64_t GetOffset() const noexcept = 0;
    };
}   // namespace sharpen

#endif