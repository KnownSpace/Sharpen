#pragma once
#ifndef _SHARPEN_QUORUM_HPP
#define _SHARPEN_QUORUM_HPP

#include <map>

#include "IQuorum.hpp"

namespace sharpen
{
    class Quorum:public sharpen::IQuorum
    {
    private:
        using Self = Quorum;

        std::map<std::uint64_t,std::unique_ptr<sharpen::IRemoteActorBuilder>> builders_;
    
        virtual sharpen::IRemoteActorBuilder *NviLookup(std::uint64_t actorId) noexcept override;

        virtual const sharpen::IRemoteActorBuilder *NviLookup(std::uint64_t actorId) const noexcept override;

        virtual void NviRegister(std::uint64_t actorId,std::unique_ptr<sharpen::IRemoteActorBuilder> builder) override;
    public:
    
        Quorum() = default;
    
        Quorum(Self &&other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept;
    
        virtual ~Quorum() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::Broadcaster CreateBroadcaster() const override;

        virtual void Remove(std::uint64_t actorId) noexcept override;
    };
}

#endif