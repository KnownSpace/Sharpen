#pragma once
#ifndef _SHARPEN_IQUORUMMAP_HPP
#define _SHARPEN_IQUORUMMAP_HPP

#include <cassert>
#include <set>

#include "IRemoteActorBuilder.hpp"
#include "Broadcaster.hpp"

namespace sharpen
{
    class IQuorum
    {
    private:
        using Self = sharpen::IQuorum;
    protected:

        virtual sharpen::IRemoteActorBuilder *NviLookup(std::uint64_t actorId) noexcept = 0;

        virtual const sharpen::IRemoteActorBuilder *NviLookup(std::uint64_t actorId) const noexcept = 0;

        virtual void NviRegister(std::uint64_t actorId,std::unique_ptr<sharpen::IRemoteActorBuilder> builder) = 0;

        virtual std::unique_ptr<sharpen::Broadcaster> NviCreateBroadcaster(std::size_t pipeline) const = 0;
    public:
    
        IQuorum() noexcept = default;
    
        IQuorum(const Self &other) noexcept = default;
    
        IQuorum(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IQuorum() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        std::unique_ptr<sharpen::Broadcaster> CreateBroadcaster() const
        {
            return this->CreateBroadcaster(1);
        }

        std::unique_ptr<sharpen::Broadcaster> CreateBroadcaster(std::size_t pipeline) const
        {
            assert(pipeline > 0);
            return this->NviCreateBroadcaster((std::max)(static_cast<std::size_t>(1),pipeline));
        }

        inline sharpen::IRemoteActorBuilder *Lookup(std::uint64_t actorId)
        {
            return this->NviLookup(actorId);
        }

        inline const sharpen::IRemoteActorBuilder *Lookup(std::uint64_t actorId) const
        {
            return this->NviLookup(actorId);
        }

        inline sharpen::IRemoteActorBuilder &Get(std::uint64_t actorId) noexcept
        {
            sharpen::IRemoteActorBuilder *builder{this->NviLookup(actorId)};
            assert(builder != nullptr);
            return *builder;
        }

        inline const sharpen::IRemoteActorBuilder &Get(std::uint64_t actorId) const noexcept
        {
            const sharpen::IRemoteActorBuilder *builder{this->NviLookup(actorId)};
            assert(builder != nullptr);
            return *builder;
        }

        inline void Register(std::uint64_t actorId,std::unique_ptr<sharpen::IRemoteActorBuilder> builder)
        {
            assert(builder != nullptr);
            this->NviRegister(actorId,std::move(builder));
        }

        virtual void Remove(std::uint64_t actorId) noexcept = 0;

        inline bool Exist(std::uint64_t actorId) const noexcept
        {
            return this->NviLookup(actorId) != nullptr;
        }

        virtual std::size_t GetSize() const noexcept = 0;

        inline std::size_t GetMajority() const noexcept
        {
            std::size_t size{this->GetSize()};
            size += 1;
            return size/2;
        }

        inline bool Empty() const noexcept
        {
            return !this->GetSize();
        }

        virtual std::set<std::uint64_t> GenerateActorsSet() const = 0;
    };
}

#endif