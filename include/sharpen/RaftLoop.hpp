#pragma once
#ifndef _SHARPEN_RAFTGROUP_HPP
#define _SHARPEN_RAFTGROUP_HPP

#include <random>

#include "RaftWrapper.hpp"
#include "TimerLoop.hpp"
#include "AsyncMutex.hpp"
#include "RaftLoopOption.hpp"

namespace sharpen
{
    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage,typename _Check = void>
    class InternalRaftLoop;

    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage>
    class InternalRaftLoop<_Id,_Member,_Log,_Application,_PersistentStorage,sharpen::EnableIf<sharpen::RaftWrapperRequires<_Id,_Member,_Log,_Application,_PersistentStorage>::Value>>:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::InternalRaftLoop<_Id,_Member,_Log,_Application,_PersistentStorage,sharpen::EnableIf<sharpen::RaftWrapperRequires<_Id,_Member,_Log,_Application,_PersistentStorage>::Value>>;
        using RaftType = sharpen::RaftWrapper<_Id,_Member,_Log,_Application,_PersistentStorage>;
        using RaftLock = sharpen::AsyncMutex;
        using VoteLock = sharpen::SpinLock;

        inline std::chrono::milliseconds GenerateElectionWaitTime() const noexcept
        {
            std::uint32_t val{this->distribution_(this->random_)};
            return std::chrono::milliseconds{val};
        }
    protected:

        virtual sharpen::TimerLoop::LoopStatus FollowerLoop() noexcept = 0;

        virtual sharpen::TimerLoop::LoopStatus LeaderLoop() noexcept = 0;
    public:

        sharpen::EventEngine *engine_;
        mutable std::minstd_rand random_;
        mutable std::uniform_int_distribution<std::uint32_t> distribution_;
    protected:

        RaftType raft_;
        std::unique_ptr<RaftLock> raftLock_;
        std::unique_ptr<VoteLock> voteLock_;
        sharpen::TimerPtr proposeTimer_;
        sharpen::TimerLoop leaderLoop_;
        sharpen::TimerLoop followerLoop_;
    public:
    
        InternalRaftLoop(sharpen::EventEngine &engine,_Id id,_PersistentStorage storage,std::shared_ptr<_Application> app,const sharpen::RaftLoopOption &option)
            :engine_(&engine)
            ,random_(option.GetRandomSeed())
            ,distribution_(static_cast<std::uint32_t>(option.GetMinElectionCycle().count()),static_cast<std::uint32_t>(option.GetMaxElectionCycle().count()))
            ,raft_(std::move(id),std::move(storage),std::move(app))
            ,raftLock_(new RaftLock{})
            ,voteLock_(new VoteLock{})
            ,proposeTimer_(option.GetTimerMaker()(*this->engine_))
            ,leaderLoop_(*this->engine_,option.GetTimerMaker()(*this->engine_),option.GetAppendEntriesCycle(),std::bind(&Self::LeaderLoop,this))
            ,followerLoop_(*this->engine_,option.GetTimerMaker()(*this->engine_),std::bind(&Self::FollowerLoop,this),std::bind(&Self::GenerateElectionWaitTime,this))
        {
            assert(option.GetMaxElectionCycle().count() > option.GetMinElectionCycle().count());
            if(!this->raftLock_ || !this->voteLock_)
            {
                throw std::bad_alloc();
            }
        }
    
        InternalRaftLoop(Self &&other) noexcept
            :engine_(other.engine_)
            ,random_(std::move(other.random_))
            ,distribution_(std::move(other.distribution_))
            ,raft_(std::move(other.raft_))
            ,raftLock_(std::move(other.raftLock_))
            ,voteLock_(std::move(other.voteLock_))
            ,proposeTimer_(std::move(other.proposeTimer_))
            ,leaderLoop_(std::move(other.leaderLoop_))
            ,followerLoop_(std::move(other.followerLoop_))
        {}
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->engine_ = other.engine_;
                this->random_ = std::move(other.random_);
                this->distribution_ = std::move(other.distribution_);
                this->raft_ = std::move(other.raft_);
                this->raftLock_ = std::move(other.raftLock_);
                this->voteLock_ = std::move(other.voteLock_);
                this->proposeTimer_ = std::move(other.proposeTimer_);
                this->leaderLoop_ = std::move(other.leaderLoop_);
                this->followerLoop_ = std::move(other.followerLoop_);
            }
            return *this;
        }
    
        virtual ~InternalRaftLoop() noexcept
        {
            this->Stop();
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void DelayFollowerCycle()
        {
            this->followerLoop_.Cancel();
        }

        inline void DelayLeaderCycle()
        {
            this->leaderLoop_.Cancel();
        }

        inline void Start()
        {
            this->followerLoop_.Start();
        }

        inline void Stop() noexcept
        {
            this->followerLoop_.Terminate();
            this->leaderLoop_.Terminate();
        }

        inline RaftLock &GetRaftLock() const noexcept
        {
            return *this->raftLock_;
        }

        inline VoteLock &GetVoteLock() const noexcept
        {
            return *this->voteLock_;
        }

        inline RaftType &Raft() noexcept
        {
            return this->raft_;
        }

        inline const RaftType &Raft() const noexcept
        {
            return this->raft_;
        }
    };

    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage>
    using RaftLoop = sharpen::InternalRaftLoop<_Id,_Member,_Log,_Application,_PersistentStorage>;
}

#endif