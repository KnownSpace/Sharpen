#pragma once
#ifndef _SHARPEN_COWSTATUSMAP_HPP
#define _SHARPEN_COWSTATUSMAP_HPP

#include <map>
#include <string>

#include "AsyncRwLock.hpp"
#include "IEventLoopGroup.hpp"
#include "IFileChannel.hpp"
#include "IStatusMap.hpp"

namespace sharpen
{
    class CowStatusMap
        : public sharpen::IStatusMap
        , public sharpen::Noncopyable
    {
    private:
        using Self = CowStatusMap;
        using Map = std::map<sharpen::ByteBuffer, sharpen::ByteBuffer>;

        std::string name_;
        std::string tempName_;
        Map map_;
        sharpen::IEventLoopGroup *loopGroup_;
        std::unique_ptr<sharpen::AsyncRwLock> lock_;

        void Load();

        void Save();

        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(
            const sharpen::ByteBuffer &key) const override;

        virtual void NviWrite(sharpen::ByteBuffer key, sharpen::ByteBuffer value) override;

        virtual void NviRemove(const sharpen::ByteBuffer &key) override;

    public:
        explicit CowStatusMap(std::string name);

        CowStatusMap(sharpen::IEventLoopGroup &loopGroup, std::string name);

        CowStatusMap(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        virtual ~CowStatusMap() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}   // namespace sharpen

#endif