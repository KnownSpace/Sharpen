#pragma once
#ifndef _SHARPEN_RAFTCONSENSUS_HPP
#define _SHARPEN_RAFTCONSENSUS_HPP

#include "Broadcaster.hpp"
#include "IConsensus.hpp"
#include "IMailReceiver.hpp"
#include "IQuorum.hpp"
#include "IRaftLogAccesser.hpp"
#include "IRaftMailBuilder.hpp"
#include "IRaftMailExtractor.hpp"
#include "IRaftSnapshotController.hpp"
#include "IStatusMap.hpp"
#include "IWorkerGroup.hpp"
#include "Noncopyable.hpp"
#include "RaftElectionRecord.hpp"
#include "RaftHeartbeatMailProvider.hpp"
#include "RaftLeaderRecord.hpp"
#include "RaftOption.hpp"
#include "RaftPrevoteRecord.hpp"
#include "RaftRole.hpp"
#include "RaftVoteRecord.hpp"
#include <map>
#include <queue>
#include <set>

namespace sharpen {
    class RaftConsensus
        : public sharpen::IConsensus
        , private sharpen::IMailReceiver
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::RaftConsensus;

        // scheduler
        sharpen::IFiberScheduler *scheduler_;

        // the id of current actor
        sharpen::ActorId id_;
        // persistent status map
        std::unique_ptr<sharpen::IStatusMap> statusMap_;
        // storage logs
        std::unique_ptr<sharpen::ILogStorage> logs_;
        std::unique_ptr<sharpen::IRaftLogAccesser> logAccesser_;
        // snapshot provider
        std::unique_ptr<sharpen::IRaftSnapshotController> snapshotController_;
        // raft option
        sharpen::RaftOption option_;
        // cache
        std::atomic_uint64_t term_;
        sharpen::RaftVoteRecord vote_;
        std::atomic_uint64_t commitIndex_;
        // role
        std::atomic<sharpen::RaftRole> role_;
        // election record
        sharpen::RaftElectionRecord electionRecord_;
        sharpen::RaftPrevoteRecord prevoteRecord_;

        // leader record
        // thread safty
        sharpen::RaftLeaderRecord leaderRecord_;

        // waiters
        std::atomic<sharpen::Future<void> *> waiter_;
        std::atomic_uint64_t advancedCount_;
        std::atomic_uint64_t reachAdvancedCount_;

        // mail builder
        std::unique_ptr<sharpen::IRaftMailBuilder> mailBuilder_;
        // mail extractor
        std::unique_ptr<sharpen::IRaftMailExtractor> mailExtractor_;
        // quorums
        std::unique_ptr<sharpen::IQuorum> peers_;
        // std::unique_ptr<sharpen::IQuorum> learners_;
        // quorum broadcasters
        std::unique_ptr<sharpen::Broadcaster> peersBroadcaster_;
        // std::unique_ptr<sharpen::Broadcaster> learnerBroadcaster_;

        // quorum heartbeat provider
        std::unique_ptr<sharpen::RaftHeartbeatMailProvider> heartbeatProvider_;
        // learner heartbeat provider


        // must be last member
        // single fiber worker
        std::unique_ptr<sharpen::IWorkerGroup> worker_;

        sharpen::Optional<std::uint64_t> LoadUint64(sharpen::ByteSlice key);

        void SetUint64(sharpen::ByteSlice key, std::uint64_t value);

        void LoadTerm();

        void LoadCommitIndex();

        void LoadVoteFor();

        std::uint64_t GetTerm() const noexcept;

        void SetTerm(std::uint64_t term);

        sharpen::RaftVoteRecord GetVote() const noexcept;

        void SetVote(sharpen::RaftVoteRecord vote);

        std::uint64_t GetLastIndex() const;

        sharpen::IRaftSnapshotProvider &GetSnapshotProvider() noexcept;

        const sharpen::IRaftSnapshotProvider &GetSnapshotProvider() const noexcept;

        sharpen::IRaftSnapshotInstaller &GetSnapshotInstaller() noexcept;

        const sharpen::IRaftSnapshotInstaller &GetSnapshotInstaller() const noexcept;

        sharpen::Optional<std::uint64_t> LookupTerm(std::uint64_t index) const;

        sharpen::Optional<std::uint64_t> LookupTermOfEntry(std::uint64_t index) const noexcept;

        bool CheckEntry(std::uint64_t index, std::uint64_t expectedTerm) const noexcept;

        void EnsureBroadcaster();

        void EnsureConfig() const;

        void OnStatusChanged();

        void RaiseElection();

        void RaisePrevote();

        void ComeToPower();

        void Abdicate();

        // vote
        sharpen::Mail OnVoteRequest(const sharpen::RaftVoteForRequest &request);

        void OnVoteResponse(const sharpen::RaftVoteForResponse &response,
                            const sharpen::ActorId &actorId);

        // prevote
        sharpen::Mail OnPrevoteRequest(const sharpen::RaftPrevoteRequest &request);

        void OnPrevoteResponse(const sharpen::RaftPrevoteResponse &response,
                               const sharpen::ActorId &actorId);

        // heartbeat
        sharpen::Mail OnHeartbeatRequest(const sharpen::RaftHeartbeatRequest &request);

        void OnHeartbeatResponse(const sharpen::RaftHeartbeatResponse &response,
                                 const sharpen::ActorId &actorId);

        // snapshot
        sharpen::Mail OnSnapshotRequest(const sharpen::RaftSnapshotRequest &request);

        void OnSnapshotResponse(const sharpen::RaftSnapshotResponse &response,
                                const sharpen::ActorId &actorId);

        void NotifyWaiter(sharpen::Future<void> *future) noexcept;

        virtual void NviWaitNextConsensus(sharpen::Future<void> &future) override;

        virtual bool NviIsConsensusMail(const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Mail NviGenerateResponse(sharpen::Mail request) override;

        sharpen::Mail DoGenerateResponse(sharpen::Mail request);

        virtual void NviReceive(sharpen::Mail mail, const sharpen::ActorId &actorId) override;

        void DoReceive(sharpen::Mail mail, sharpen::ActorId actorId);

        virtual void NviConfiguratePeers(
            std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum *)> configurater)
            override;

        void DoConfiguratePeers(
            std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum *)> configurater);

        void DoClosePeers();

        sharpen::WriteLogsResult DoWrite(const sharpen::LogBatch *logs);

        virtual sharpen::WriteLogsResult NviWrite(const sharpen::LogBatch &logs) override;

        virtual void NviDropLogsUntil(std::uint64_t index) override;

        void DoAdvance();

    public:
        constexpr static sharpen::ByteSlice voteKey{"vote", 4};

        constexpr static sharpen::ByteSlice termKey{"term", 4};

        constexpr static sharpen::ByteSlice lastAppiledKey{"lastAppiled", 11};

        RaftConsensus(const sharpen::ActorId &id,
                      std::unique_ptr<sharpen::IStatusMap> statusMap,
                      std::unique_ptr<sharpen::ILogStorage> logs,
                      std::unique_ptr<sharpen::IRaftLogAccesser> logAccesser,
                      std::unique_ptr<sharpen::IRaftSnapshotController> snapshotController,
                      const sharpen::RaftOption &option);

        RaftConsensus(const sharpen::ActorId &id,
                      std::unique_ptr<sharpen::IStatusMap> statusMap,
                      std::unique_ptr<sharpen::ILogStorage> logs,
                      std::unique_ptr<sharpen::IRaftLogAccesser> logAccesser,
                      std::unique_ptr<sharpen::IRaftSnapshotController> snapshotController,
                      const sharpen::RaftOption &option,
                      sharpen::IFiberScheduler &scheduler);

        virtual ~RaftConsensus() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void PrepareMailBuilder(std::unique_ptr<sharpen::IRaftMailBuilder> builder) noexcept;

        void PrepareMailExtractor(std::unique_ptr<sharpen::IRaftMailExtractor> extractor) noexcept;

        virtual void Advance() override;

        virtual bool Writable() const override;

        virtual bool Changable() const override;

        virtual const sharpen::ILogStorage &ImmutableLogs() const noexcept override;

        inline virtual sharpen::IMailReceiver &GetReceiver() noexcept override {
            return *this;
        }

        inline virtual const sharpen::IMailReceiver &GetReceiver() const noexcept override {
            return *this;
        }

        // inline const sharpen::IQuorum &ImmutableQuorum() const noexcept
        // {
        //     assert(this->quorum_ != nullptr);
        //     return *this->quorum_;
        // }

        virtual sharpen::Optional<sharpen::ActorId> GetWriterId() const noexcept override;

        // void ConfigurateLearners(std::function<void(sharpen::IQuorum&)> configurater);

        // template<typename _Fn,typename ..._Args,typename _Check =
        // sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,sharpen::IQuorum&,_Args...>::Value>>
        // inline void ConfigurateLearners(_Fn &&fn,_Args &&...args)
        // {
        //     std::function<void(sharpen::IQuorum&)>
        //     config{std::bind(std::forward<_Fn>(fn),std::placeholders::_1,std::forward<_Args>(args)...)};
        //     this->ConfigurateLearners(config);
        // }

        std::uint64_t GetCommitIndex() const noexcept override;

        void ClosePeers() override;
    };
}   // namespace sharpen

#endif