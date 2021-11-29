#pragma once
#ifndef _SHARPEN_RAFTSTATEMACHINE_HPP
#define _SHARPEN_RAFTSTATEMACHINE_HPP

#include <unordered_map>
#include <vector>
#include <iterator>

#include "RaftRole.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "Future.hpp"
#include "Quorum.hpp"
#include "CompressedPair.hpp"
#include "Option.hpp"
#include "RaftConcepts.hpp"

namespace sharpen
{

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member>
    using RaftStateMachineRequires = sharpen::BoolType<sharpen::IsRaftLog<_Log>::Value
                                                        && sharpen::IsRaftMember<_Id,_Member>::Value
                                                        && sharpen::IsRaftPersistenceStorage<_PersistenceStorage,_Log,_Id>::Value>;

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member,typename _Check = void>
    class InternalRaftStateMachine;

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member>
    class InternalRaftStateMachine<_Id,_Log,_Commiter,_PersistenceStorage,_Member,sharpen::EnableIf<sharpen::RaftStateMachineRequires<_Id,_Log,_Commiter,_PersistenceStorage,_Member>::Value>>:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MemberMap = std::unordered_map<_Id,_Member>;

        _Id selfId_;
        sharpen::Option<_Id> leaderId_;

        //persistence status
        //current term
        //logs[]
        //voted for
        _PersistenceStorage pm_;

        //volatile status
        //current role and commiter
        sharpen::CompressedPair<_Commiter,sharpen::RaftRole> rolePair_;
        //commit index
        sharpen::Uint64 commitIndex_;
        //applied index
        sharpen::Uint64 lastApplied_;

        //leader voltile status
        //next index and match index
        MemberMap members_;

        //candidate voltile status
        std::atomic_uint64_t votes_;

        void SetRole(sharpen::RaftRole role) noexcept
        {
            this->rolePair_.Second() = role;
        }

        _PersistenceStorage &PersistenceStorage() noexcept
        {
            return this->pm_;
        }

        const _PersistenceStorage &PersistenceStorage() const noexcept
        {
            return this->pm_;
        }

        void SetCurrentTerm(sharpen::Uint64 term)
        {
            this->PersistenceStorage().SetCurrentTerm(term);
        }

        void SetVotedFor(const _Id &id)
        {
            this->PersistenceStorage().SetVotedFor(id);
        }

        void ConvertFollower()
        {
            this->votes_ = 0;
            this->SetRole(sharpen::RaftRole::Follower);
        }
    public:
        explicit InternalRaftStateMachine(_Id id,_PersistenceStorage pm)
            :selfId_(std::move(id))
            ,pm_(std::move(pm))
            ,rolePair_()
            ,commitIndex_(0)
            ,lastApplied_(0)
            ,members_()
            ,votes_(0)
        {
            //set role
            this->SetRole(sharpen::RaftRole::Follower);
            //set pm
            this->PersistenceStorage() = std::move(pm);
        }

        sharpen::Uint64 LastIndex() const noexcept
        {
            if(this->PersistenceStorage().LogIsEmpty())
            {
                return 0;
            }
            return this->PersistenceStorage().LastLog().GetIndex();
        }

        sharpen::Uint64 LastTerm() const noexcept
        {
            if(this->PersistenceStorage().LogIsEmpty())
            {
                return 0;
            }
            return this->PersistenceStorage().LastLog().GetTerm();
        }

        MemberMap &Members() noexcept
        {
            return this->members_;
        }

        const MemberMap &Members() const noexcept
        {
            return this->members_;
        }

        sharpen::RaftRole GetRole() const noexcept
        {
            return this->rolePair_.Second();
        }

        _Commiter &Commiter() noexcept
        {
            return this->rolePair_.First();
        }

        const _Commiter &Commiter() const noexcept
        {
            return this->rolePair_.First();
        }

        template<typename _LogIterator,typename _Check = sharpen::EnableIf<sharpen::IsRaftLogIterator<_LogIterator>::Value>>
        bool AppendEntries(_LogIterator begin,_LogIterator end,const _Id &leaderId,sharpen::Uint64 leaderTerm,sharpen::Uint64 preLogIndex,sharpen::Uint64 preLogTerm,sharpen::Uint64 leaderCommit)
        {
            if(leaderTerm < this->CurrentTerm())
            {
                return false;
            }
            if(this->CurrentTerm() < leaderTerm)
            {
                this->ConvertFollower();
                this->SetCurrentTerm(leaderTerm);
            }
            //reset voted for
            this->PersistenceStorage().ResetVotedFor();
            this->leaderId_.Construct(leaderId);
            if(!this->PersistenceStorage().LogIsEmpty())
            {
                if(this->PersistenceStorage().ContainLog(preLogIndex))
                {
                    const _Log &log = this->PersistenceStorage().FindLog(preLogIndex);
                    if(log.GetTerm() != preLogTerm)
                    {
                        return false;
                    }
                    this->PersistenceStorage().EraseLogAfter(preLogIndex);
                }
            }
            for (auto ite = begin; ite != end; ++ite)
            {
                this->PersistenceStorage().PushLog(*begin);
            }
            //update commit index
            //prepare to commit
            if(leaderCommit > this->commitIndex_)
            {
                this->commitIndex_ = leaderCommit;
            }
            //commit logs
            while (begin != end)
            {
                this->Commiter().Commit(*begin);
                ++begin;
            }
            return true;
        }

        sharpen::Uint64 RaiseElection()
        {
            this->votes_ = 0;
            this->SetRole(sharpen::RaftRole::Candidate);
            this->PersistenceStorage().SetVotedFor(this->selfId_);
            return this->PersistenceStorage().AddCurrentTerm();
        }

        void GetVote(sharpen::Uint64 vote)
        {
            this->votes_ += vote;
        }

        bool StopElection()
        {
            if (this->GetRole() == sharpen::RaftRole::Candidate && this->votes_ >= this->Members().size()/2)
            {
                this->SetRole(sharpen::RaftRole::Leader);
                return true;
            }
            return false;
        }

        bool RequestVote(sharpen::Uint64 candidateTerm,const _Id &candidateId,sharpen::Uint64 lastLogIndex,sharpen::Uint64 lastLogTerm)
        {
            if(this->CurrentTerm() > candidateTerm)
            {
                return false;
            }
            if(this->CurrentTerm() < candidateTerm)
            {
                this->ConvertFollower();
                this->SetCurrentTerm(candidateTerm);
                this->PersistenceStorage().ResetVotedFor();
            }
            if((!this->PersistenceStorage().IsVotedFor() || this->PersistenceStorage().GetVotedFor() == candidateId) && lastLogIndex >= this->LastIndex() && lastLogTerm >= this->LastTerm())
            {
                this->ConvertFollower();
                this->PersistenceStorage().SetVotedFor(candidateId);
                return true;
            }
            return false;
        }

        sharpen::Uint64 CurrentTerm() const noexcept
        {
            return this->PersistenceStorage().GetCurrentTerm();
        }

        const _Id &SelfId() const noexcept
        {
            return this->selfId_;
        }

        //return true if we continue
        //this impl is optional
        template<typename _UCommiter = _Commiter,typename _Check = decltype(std::declval<_UCommiter>().InstallSnapshot(std::declval<_PersistenceStorage>(),0,0,0,0,nullptr,false))>
        bool InstallSnapshot(const _Id &leaderId,sharpen::Uint64 leaderTerm,sharpen::Uint64 lastIncludedIndex,sharpen::Uint64 lastIncludedTerm,sharpen::Uint64 offset,const char *data,bool done)
        {
            if (this->CurrentTerm() > leaderTerm)
            {
                return false;
            }
            if(this->CurrentTerm() < leaderTerm)
            {
                this->ConvertFollower();
                this->SetCurrentTerm(leaderTerm);
            }
            this->leaderId_.Construct(leaderId);
            this->Commiter().InstallSnapshot(this->PersistenceStorage(),leaderTerm,lastIncludedIndex,lastIncludedTerm,offset,data,done);
            return !done;
        }

        bool KnowLeader() const noexcept
        {
            return this->leaderId_.HasValue();
        }

        const _Id &LeaderId() const noexcept
        {
            return this->leaderId_;
        }

        void ResetLeader()
        {
            this->leaderId_ = sharpen::NullOpt;
        }

        ~InternalRaftStateMachine() noexcept = default;
    };

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member>
    using RaftStateMachine = sharpen::InternalRaftStateMachine<_Id,_Log,_Commiter,_PersistenceStorage,_Member>;
}

#endif