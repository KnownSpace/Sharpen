#pragma once
#ifndef _SHARPEN_RAFTHEARTBEATMAILPROVIDER_HPP
#define _SHARPEN_RAFTHEARTBEATMAILPROVIDER_HPP

#include "ILogStorage.hpp"
#include "IMailProvider.hpp"
#include "IRaftLogAccesser.hpp"
#include "IRaftMailBuilder.hpp"
#include "IRaftSnapshotProvider.hpp"
#include "Noncopyable.hpp"
#include "Optional.hpp"
#include "RaftReplicatedState.hpp"
#include <atomic>
#include <cassert>
#include <limits>
#include <map>


namespace sharpen {
    class RaftHeartbeatMailProvider
        : public sharpen::IMailProvider
        , public sharpen::Noncopyable {
    private:
        using Self = sharpen::RaftHeartbeatMailProvider;

        static constexpr std::size_t defaultBatchSize_{8};

        static constexpr std::size_t maxEntiresSize_{(std::numeric_limits<std::int32_t>::max)()};

        static constexpr std::size_t defaultEntiresSize_{maxEntiresSize_};

        static constexpr std::size_t minEntiresSize_{4*1024};

        sharpen::ActorId id_;
        const sharpen::IRaftMailBuilder *builder_;
        const sharpen::ILogStorage *logs_;
        sharpen::IRaftLogAccesser *logAccesser_;
        sharpen::IRaftSnapshotProvider *snapshotProvider_;
        // max size of each batch
        std::size_t batchSize_;
        // max length of each entires
        std::size_t entiresSize_;
        mutable std::map<sharpen::ActorId, sharpen::RaftReplicatedState> states_;
        std::uint64_t term_;
        std::uint64_t round_;
        std::atomic_uint64_t commitIndex_;

        sharpen::RaftReplicatedState *LookupMutableState(
            const sharpen::ActorId &actorId) const noexcept;

        sharpen::Mail ProvideSnapshotRequest(sharpen::RaftReplicatedState *state) const;

        void ReComputeCommitIndex() noexcept;

        sharpen::Optional<std::uint64_t> LookupTerm(std::uint64_t index) const noexcept;

    public:
        RaftHeartbeatMailProvider(const sharpen::ActorId &id,
                                  const sharpen::IRaftMailBuilder &builder,
                                  const sharpen::ILogStorage &log,
                                  sharpen::IRaftLogAccesser &logAccesser,
                                  sharpen::IRaftSnapshotProvider *snapshotProvider);

        RaftHeartbeatMailProvider(const sharpen::ActorId &id,
                                  const sharpen::IRaftMailBuilder &builder,
                                  const sharpen::ILogStorage &log,
                                  sharpen::IRaftLogAccesser &logAccesser,
                                  sharpen::IRaftSnapshotProvider *snapshotProvider,
                                  std::uint32_t batchSize);

        RaftHeartbeatMailProvider(const sharpen::ActorId &id,
                                  const sharpen::IRaftMailBuilder &builder,
                                  const sharpen::ILogStorage &log,
                                  sharpen::IRaftLogAccesser &logAccesser,
                                  sharpen::IRaftSnapshotProvider *snapshotProvider,
                                  std::uint32_t batchSize,
                                  std::uint32_t entiresSize);

        RaftHeartbeatMailProvider(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        virtual ~RaftHeartbeatMailProvider() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void SetCommitIndex(std::uint64_t index) noexcept;

        virtual sharpen::Mail Provide(const sharpen::ActorId &actorId) const;

        void Register(const sharpen::ActorId &actorId);

        const sharpen::RaftReplicatedState *LookupState(
            const sharpen::ActorId &actorId) const noexcept;

        sharpen::Optional<std::uint64_t> LookupMatchIndex(
            const sharpen::ActorId &actorId) const noexcept;

        void ForwardState(const sharpen::ActorId &actorId, std::uint64_t index) noexcept;

        void BackwardState(const sharpen::ActorId &actorId, std::uint64_t index) noexcept;

        sharpen::Optional<std::uint64_t> GetSynchronizedIndex() const noexcept;

        void PrepareTerm(std::uint64_t term) noexcept;

        void PrepareRound(std::uint64_t round) noexcept;

        sharpen::Mail ProvideSynchronizedMail() const;

        void RemoveState(const sharpen::ActorId &actorId) noexcept;

        std::size_t GetSize() const noexcept;

        bool Empty() const noexcept;

        void Clear() noexcept;

        std::uint64_t GetCommitIndex() const noexcept;

        inline sharpen::IRaftLogAccesser &LogAccesser() noexcept {
            assert(this->logAccesser_ != nullptr);
            return *this->logAccesser_;
        }

        inline const sharpen::IRaftLogAccesser &LogAccesser() const noexcept {
            assert(this->logAccesser_ != nullptr);
            return *this->logAccesser_;
        }
    };
}   // namespace sharpen

#endif