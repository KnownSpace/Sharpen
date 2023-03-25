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

namespace sharpen
{
    class RaftHeartbeatMailProvider:public sharpen::IMailProvider,public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::RaftHeartbeatMailProvider;
    
        static constexpr std::size_t defaultBatchSize_{5};

        static constexpr std::size_t defaultPipelineLength_{1};

        std::uint64_t id_;
        const sharpen::IRaftMailBuilder *builder_;
        const sharpen::ILogStorage *log_;
        // sharpen::IRaftSnapshotProvider *snapshotProvider_;
        std::size_t batchSize_;
        std::size_t pipelineLength_;
        std::map<std::uint64_t,std::uint64_t> nextIndexs_;
        std::map<std::uint64_t,std::uint64_t> matchIndexs_;
        // std::map<std::uint64_t,std::unique_ptr<sharpen::IRaftSnapshot>> snapshots_;
        std::uint64_t term_;
        // std::uint64_t commitIndex_;
        mutable std::uint64_t commitIndex_;
    public:
    
        RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log);

        RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log,std::uint16_t batchSize,std::uint16_t pipelineLength);
    
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

        sharpen::Optional<std::uint64_t> LookupIndex(std::uint64_t actorId) const noexcept;

        void SetIndex(std::uint64_t actorId,std::uint64_t index);

        sharpen::Optional<std::uint64_t> GetSynchronizedIndex() const noexcept;

        sharpen::Mail ProvideSynchronizedMail() const;

        void RemoveIndex(std::uint64_t actorId) noexcept;

        std::size_t GetSize() const noexcept;

        bool Empty() const noexcept;

        std::size_t GetCommitIndex() const noexcept;
    };
}

#endif