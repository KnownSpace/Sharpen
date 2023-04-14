#pragma once
#ifndef _SHARPEN_RAFTHEARTBEATMAILPROVIDER_HPP
#define _SHARPEN_RAFTHEARTBEATMAILPROVIDER_HPP

#include <map>

#include "IMailProvider.hpp"
#include "Optional.hpp"
#include "IRaftMailBuilder.hpp"
#include "ILogStorage.hpp"
#include "IRaftSnapshotProvider.hpp"
#include "Noncopyable.hpp"
#include "RaftReplicatedState.hpp"

namespace sharpen
{
    class RaftHeartbeatMailProvider:public sharpen::IMailProvider,public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::RaftHeartbeatMailProvider;
    
        static constexpr std::size_t defaultBatchSize_{8};

        // static constexpr std::size_t defaultPipelineLength_{64};

        std::uint64_t id_;
        const sharpen::IRaftMailBuilder *builder_;
        const sharpen::ILogStorage *logs_;
        sharpen::IRaftSnapshotProvider *snapshotProvider_;
        //max size of each batch
        std::size_t batchSize_;
        mutable std::map<std::uint64_t,sharpen::RaftReplicatedState> states_;
        std::uint64_t term_;
        std::uint64_t commitIndex_;

        sharpen::RaftReplicatedState *LookupMutableState(std::uint64_t actorId) const noexcept;

        sharpen::Mail ProvideSnapshotRequest(sharpen::RaftReplicatedState *state) const;

        void ReComputeCommitIndex() noexcept;
    public:
    
        RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log,sharpen::IRaftSnapshotProvider *snapshotProvider);

        RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log,sharpen::IRaftSnapshotProvider *snapshotProvider,std::uint32_t batchSize);
    
        RaftHeartbeatMailProvider(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        virtual ~RaftHeartbeatMailProvider() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void PrepareTerm(std::uint64_t term) noexcept;

        void SetCommitIndex(std::uint64_t index) noexcept;

        virtual sharpen::Mail Provide(std::uint64_t actorId) const;

        void Register(std::uint64_t actorId);

        const sharpen::RaftReplicatedState *LookupState(std::uint64_t actorId) const noexcept;

        sharpen::Optional<std::uint64_t> LookupMatchIndex(std::uint64_t actorId) const noexcept;

        void ForwardState(std::uint64_t actorId,std::uint64_t index) noexcept;

        void BackwardState(std::uint64_t actorId,std::uint64_t index) noexcept;

        sharpen::Optional<std::uint64_t> GetSynchronizedIndex() const noexcept;

        sharpen::Mail ProvideSynchronizedMail() const;

        void RemoveState(std::uint64_t actorId) noexcept;

        std::size_t GetSize() const noexcept;

        bool Empty() const noexcept;

        void Clear() noexcept;

        std::size_t GetCommitIndex() const noexcept;
    };
}

#endif