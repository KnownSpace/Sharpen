#pragma once
#ifndef _SHARPEN_WALSTATUSMAP_HPP
#define _SHARPEN_WALSTATUSMAP_HPP

#include <map>
#include <string>

#include "IStatusMap.hpp"
#include "IFileChannel.hpp"
#include "AsyncRwLock.hpp"

namespace sharpen
{
    class WalStatusMap:public sharpen::IStatusMap,public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::WalStatusMap;
        using Map = std::map<sharpen::ByteBuffer,sharpen::ByteBuffer>;

        constexpr static std::uint8_t writeTag_{0};
        
        constexpr static std::uint8_t removeTag_{1};

        constexpr static std::size_t limitFactor_{2};
        
        std::string name_;
        std::string tempName_;
        sharpen::FileChannelPtr channel_;
        sharpen::IEventLoopGroup *loopGroup_;
        Map map_;
        std::unique_ptr<sharpen::AsyncRwLock> lock_;
        std::uint64_t offset_;
        std::size_t contentSize_;

        bool Insert(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        bool Erase(const sharpen::ByteBuffer &key) noexcept;

        void Load();

        std::size_t ComputeContentSize() const noexcept;

        void RebuildFile();

        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(const sharpen::ByteBuffer &key) const override;

        virtual void NviWrite(sharpen::ByteBuffer key,sharpen::ByteBuffer value) override;

        virtual void NviRemove(const sharpen::ByteBuffer &key) override;

    public:
    
        explicit WalStatusMap(std::string name);
    
        WalStatusMap(sharpen::IEventLoopGroup &loopGroup,std::string name);
    
        WalStatusMap(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        virtual ~WalStatusMap() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif