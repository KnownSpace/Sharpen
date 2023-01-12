#pragma once
#ifndef _SHARPEN_RAFTCONSENSUS_HPP
#define _SHARPEN_RAFTCONSENSUS_HPP

#include <map>
#include <queue>

#include "IConsensus.hpp"
#include "IStatusMap.hpp"
#include "Noncopyable.hpp"
#include "Broadcaster.hpp"
#include "IRaftMailBuilder.hpp"
#include "IRaftMailExtractor.hpp"
#include "IMailReceiver.hpp"
#include "IQuorum.hpp"
#include "TypeTraits.hpp"
#include "IWorkerGroup.hpp"
#include "RaftRole.hpp"
#include "RaftVoteRecord.hpp"
#include "RaftElectionRecord.hpp"
#include "ConsensusWaiter.hpp"

namespace sharpen
{
    class RaftConsensus:public sharpen::IConsensus,private sharpen::IMailReceiver,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::RaftConsensus;
    
        //scheduler
        sharpen::IFiberScheduler *scheduler_;

        //the id of current actor
        std::uint64_t id_;
        //persistent status map
        std::unique_ptr<sharpen::IStatusMap> statusMap_;
        //storage logs
        std::unique_ptr<sharpen::ILogStorage> logs_;
        //cache
        std::uint64_t term_;
        std::uint64_t commitIndex_;
        sharpen::RaftVoteRecord vote_;
        //role
        std::atomic<sharpen::RaftRole> role_;
        //election record
        sharpen::RaftElectionRecord electionRecord_;
        
        //waiters
        std::priority_queue<sharpen::ConsensusWaiter> waiters_;
        std::atomic_uint64_t advancedCount_;

        //mail builder
        std::unique_ptr<sharpen::IRaftMailBuilder> mailBuilder_;
        //mail extractor
        std::unique_ptr<sharpen::IRaftMailExtractor> mailExtractor_;
        //quorum
        std::unique_ptr<sharpen::IQuorum> quorum_;
        //learner quorum
        // std::unique_ptr<sharpen::IQuorum> learners_;
        //quorum broadcaster
        std::unique_ptr<sharpen::Broadcaster> quorumBroadcaster_;
        // std::unique_ptr<sharpen::Broadcaster> learnerBroadcaster_;
        
        std::map<std::uint64_t,std::uint64_t> nextIndex_;

        //must be last member
        //single fiber worker
        std::unique_ptr<sharpen::IWorkerGroup> worker_;

        sharpen::Optional<std::uint64_t> LoadUint64(sharpen::ByteSlice key);

        void SetUint64(sharpen::ByteSlice key,std::uint64_t value);

        void LoadTerm();

        void LoadCommitIndex();

        void LoadVoteFor();

        std::uint64_t GetTerm() const noexcept;

        void SetTerm(std::uint64_t term);

        sharpen::RaftVoteRecord GetVote() const noexcept;

        void SetVote(sharpen::RaftVoteRecord vote);

        std::uint64_t GetId() const noexcept;

        std::uint64_t GetCommitIndex() const noexcept;

        void EnsureBroadcaster();

        void OnStatusChanged();

        void RaiseElection();

        void NotifyWaiter(sharpen::ConsensusWaiter waiter) noexcept;

        sharpen::Mail OnVoteRequest(const sharpen::RaftVoteForRequest &request);

        void OnVoteResponse(const sharpen::RaftVoteForResponse &response,std::uint64_t actorId);

        void DoStatusChanged(std::uint64_t advancedCount,sharpen::ConsensusWaiter waiter);

        virtual void NviStatusChanged(sharpen::Future<std::uint64_t> &future,std::uint64_t minIndex) override;

        virtual bool NviIsConsensusMail(const sharpen::Mail &mail) const noexcept;

        virtual sharpen::Mail NviGenerateResponse(sharpen::Mail request) override;

        virtual void NviReceive(sharpen::Mail mail,std::uint64_t actorId) override;

        std::uint64_t DoWrite(sharpen::ILogBatch *rawLogs);

        virtual std::uint64_t NviWrite(std::unique_ptr<sharpen::ILogBatch> logs) override;

        virtual void NviDropLogsUntil(std::uint64_t index) override;

        void DoConfigurateQuorum(std::function<std::unique_ptr<sharpen::IQuorum>(std::unique_ptr<sharpen::IQuorum>)> configurater);

        void DoAdvance();

        void DoReceive(sharpen::Mail mail,std::uint64_t actorId);

        sharpen::Mail DoGenerateResponse(sharpen::Mail request);

        void EnsureConfig() const;
    public:
        constexpr static sharpen::ByteSlice voteKey{"vote",4};

        constexpr static sharpen::ByteSlice termKey{"term",4};

        constexpr static sharpen::ByteSlice lastAppiledKey{"lastAppiled",11};

        RaftConsensus(std::uint64_t id,std::unique_ptr<sharpen::IStatusMap> statusMap,std::unique_ptr<sharpen::ILogStorage> logs,bool isLearner);

        RaftConsensus(std::uint64_t id,std::unique_ptr<sharpen::IStatusMap> statusMap,std::unique_ptr<sharpen::ILogStorage> logs,bool isLearner,sharpen::IFiberScheduler &scheduler);
    
        virtual ~RaftConsensus() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void SetMailBuilder(std::unique_ptr<sharpen::IRaftMailBuilder> builder) noexcept;

        void SetMailExtractor(std::unique_ptr<sharpen::IRaftMailExtractor> extractor) noexcept;

        virtual void Advance() override;

        virtual bool Writable() const override;

        virtual std::unique_ptr<sharpen::ILogBatch> CreateLogBatch() const override;

        virtual bool Changable() const override;

        virtual const sharpen::ILogStorage &ImmutableLogs() const noexcept override;

        inline virtual sharpen::IMailReceiver &GetReceiver() noexcept
        {
            return *this;
        }

        inline virtual const sharpen::IMailReceiver &GetReceiver() const noexcept
        {
            return *this;
        }

        inline const sharpen::IQuorum &ImmutableQuorum() const noexcept
        {
            assert(this->quorum_ != nullptr);
            return *this->quorum_;
        }

        void ConfigurateQuorum(std::function<std::unique_ptr<sharpen::IQuorum>(std::unique_ptr<sharpen::IQuorum>)> configurater);

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<std::unique_ptr<sharpen::IQuorum>,_Fn,std::unique_ptr<sharpen::IQuorum>,_Args...>::Value>>
        inline void ConfigurateQuorum(_Fn &&fn,_Args &&...args)
        {
            std::function<std::unique_ptr<sharpen::IQuorum>(std::unique_ptr<sharpen::IQuorum>)> config{std::bind(std::forward<_Fn>(fn),std::placeholders::_1,std::forward<_Args>(args)...)};
            this->ConfigurateQuorum(config);
        }

        // void ConfigurateLearners(std::function<void(sharpen::IQuorum&)> configurater);

        // template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,sharpen::IQuorum&,_Args...>::Value>>
        // inline void ConfigurateLearners(_Fn &&fn,_Args &&...args)
        // {
        //     std::function<void(sharpen::IQuorum&)> config{std::bind(std::forward<_Fn>(fn),std::placeholders::_1,std::forward<_Args>(args)...)};
        //     this->ConfigurateLearners(config);
        // }
    };
}

#endif