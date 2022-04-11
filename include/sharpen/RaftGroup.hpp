#pragma once
#ifndef _SHARPEN_RAFTGROUP_HPP
#define _SHARPEN_RAFTGROUP_HPP

#include <random>

#include "RaftWrapper.hpp"
#include "TimerLoop.hpp"
#include "AsyncMutex.hpp"
#include "RaftGroupOption.hpp"

namespace sharpen
{
    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage,typename _Check = void>
    class InternalRaftGroup;

    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage>
    class InternalRaftGroup<_Id,_Member,_Log,_Application,_PersistentStorage,sharpen::RaftWrapperRequires<_Id,_Member,_Log,_Application,_PersistentStorage>>
    {
    private:
        using Self = sharpen::InternalRaftGroup<_Id,_Member,_Log,_Application,_PersistentStorage,sharpen::RaftWrapperRequires<_Id,_Member,_Log,_Application,_PersistentStorage>>;
        using RaftType = sharpen::RaftWrapper<_Id,_Member,_Log,_Application,_PersistentStorage>;
        using RaftLock = sharpen::AsyncMutex;
        using VoteLock = sharpen::SpinLock;

        inline std::chrono::milliseconds GenerateElectionWaitTime() const noexcept
        {
            std::uint32_t val{this->distribution_(this->random_)};
            return std::chrono::milliseconds{val};
        }
    protected:

        virtual sharpen::TimerLoop FollowerLoop() noexcept = 0;

        virtual sharpen::TimerLoop LeaderLoop() noexcept = 0;
    public:

        sharpen::EventEngine *engine_;
        mutable std::minstd_rand random_;
        std::uniform_int_distribution<std::uint32_t> distribution_;
    protected:

        RaftType raft_;
        std::unique_ptr<RaftLock> raftLock_;
        std::unique_ptr<VoteLock> voteLock_;
        sharpen::TimerLoop leaderLoop_;
        sharpen::TimerLoop followerLoop_;
    public:
    
        InternalRaftGroup(sharpen::EventEngine &engine,_Id id,_PersistentStorage storage,std::shared_ptr<_Application> app,const sharpen::RaftGroupOption &option)
            :engine_(&engine)
            ,random_(option.GetRandomSeed())
            ,distribution_(option.GetMinElectionWaitTime(),option.GetMaxElectionWaitTime())
            ,raft_(std::move(id),std::move(storage),std::move(app))
            ,raftLock_(new RaftLock{})
            ,voteLock_(new VoteLock{})
            ,leaderLoop_(*this->engine_,option.GetTimerMaker()(*this->engine_),std::chrono::milliseconds{option.GetAppendWaitTime()},std::bind(&Self::LeaderLoop,this))
            ,followerLoop_(*this->engine_,option.GetTimerMaker()(*this->engine_),std::bind(&Self::FollowerLoop,this),std::bind(&Self::GenerateElectionWaitTime,this))
        {
            if(!this->raftLock_ || !this->voteLock_)
            {
                throw std::bad_alloc();
            }
        }
    
        InternalRaftGroup(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~InternalRaftGroup() noexcept
        {
            this->Stop();
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void Tick()
        {
            if(this->raft_.GetRole() == sharpen::RaftRole::Follower)
            {
                return this->followerLoop_.Cancel();
            }
            return this->leaderLoop_.Cancel();
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
    using RaftGroup = sharpen::InternalRaftGroup<_Id,_Member,_Log,_Application,_PersistentStorage>;
}

#endif