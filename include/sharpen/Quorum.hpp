#pragma once
#ifndef _SHARPEN_QUORUM_HPP
#define _SHARPEN_QUORUM_HPP

#include "IQuorum.hpp"
#include <map>

namespace sharpen {
    class Quorum : public sharpen::IQuorum {
    private:
        using Self = Quorum;

        std::map<sharpen::ActorId, std::unique_ptr<sharpen::IRemoteActorBuilder>> builders_;

        virtual sharpen::IRemoteActorBuilder *NviLookup(
            const sharpen::ActorId &actorId) noexcept override;

        virtual const sharpen::IRemoteActorBuilder *NviLookup(
            const sharpen::ActorId &actorId) const noexcept override;

        virtual void NviRegister(const sharpen::ActorId &actorId,
                                 std::unique_ptr<sharpen::IRemoteActorBuilder> builder) override;

        virtual std::unique_ptr<sharpen::Broadcaster> NviCreateBroadcaster(
            std::size_t pipeline) const override;

    public:
        Quorum() = default;

        Quorum(Self &&other) noexcept = default;

        Self &operator=(Self &&other) noexcept;

        virtual ~Quorum() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void Remove(const sharpen::ActorId &actorId) noexcept override;

        inline virtual std::size_t GetSize() const noexcept override {
            return this->builders_.size();
        }

        virtual std::set<sharpen::ActorId> GenerateActorsSet() const override;
    };
}   // namespace sharpen

#endif