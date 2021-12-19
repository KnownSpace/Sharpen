#pragma once
#ifndef _SHARPEN_RAFTSTATEMACHINE_HPP
#define _SHARPEN_RAFTSTATEMACHINE_HPP

#include <unordered_map>
#include <vector>
#include <iterator>
#include <memory>

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

        void BiggerTerm(sharpen::Uint64 term)
        {
            this->ConvertFollower();
            this->SetCurrentTerm(term);
        }
    public:
        InternalRaftStateMachine(_Id id,_PersistenceStorage pm)
            :InternalRaftStateMachine(std::move(id),std::move(pm),_Commiter{})
        {}

        InternalRaftStateMachine(_Id id,_PersistenceStorage pm,_Commiter commiter)
            :selfId_(std::move(id))
            ,pm_(std::move(pm))
            ,rolePair_(std::move(commiter),sharpen::RaftRole::Follower)
            ,commitIndex_(0)
            ,lastApplied_(0)
            ,members_()
            ,votes_(0)
        {}

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

        sharpen::Size MemberMajority() const noexcept
        {
            return (this->Members().size())/2;
        }

        void ReactNewTerm(sharpen::Uint64 term)
        {
            if(term > this->CurrentTerm())
            {
                this->BiggerTerm(term);
            }
        }

        template<typename _LogIterator,typename _Check = sharpen::EnableIf<sharpen::IsRaftLogIterator<_LogIterator>::Value>>
        bool AppendEntries(_LogIterator begin,_LogIterator end,const _Id &leaderId,sharpen::Uint64 leaderTerm,sharpen::Uint64 preLogIndex,sharpen::Uint64 preLogTerm,sharpen::Uint64 leaderCommit)
        {
            //if is old leader
            if(leaderTerm < this->CurrentTerm())
            {
                return false;
            }
            //update term
            if(this->CurrentTerm() < leaderTerm)
            {
                this->BiggerTerm(leaderTerm);
                this->leaderId_.Construct(leaderId);
            }
            else if(this->GetRole() == sharpen::RaftRole::Leader)
            {
                return false;
            }
            //access denied
            else if(this->KnowLeader())
            {
                if(this->LeaderId() != leaderId)
                {
                    return false;
                }
            }
            //new leader
            else
            {
                this->leaderId_.Construct(leaderId);
            }
            //reset voted for
            this->PersistenceStorage().ResetVotedFor();
            //check logs
            if(!this->PersistenceStorage().LogIsEmpty())
            {
                if(this->PersistenceStorage().ContainLog(preLogIndex))
                {
                    const _Log &log = this->PersistenceStorage().GetLog(preLogIndex);
                    if(log.GetTerm() != preLogTerm)
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else if(preLogIndex != 0)
            {
                return false;
            }
            bool token{true};
            //erase logs if term diff with leader
            for (auto ite = begin; ite != end; ++ite)
            {
                //skip logs that already commited
                if(ite->GetIndex() > this->commitIndex_)
                {
                    if(token && !this->PersistenceStorage().LogIsEmpty() && this->PersistenceStorage().ContainLog(ite->GetIndex()))
                    {
                        //check log
                        //if log term != leader
                        if(this->PersistenceStorage().GetLog(ite->GetIndex()).GetTerm() != ite->GetTerm())
                        {
                            //remove logs [index,end)
                            this->PersistenceStorage().EraseLogAfter(ite->GetIndex() - 1);
                            //skip check
                            token = false;
                            //push log
                            this->PersistenceStorage().PushLog(*ite);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    this->PersistenceStorage().PushLog(*ite);
                }
            }
            //commit logs
            sharpen::Uint64 oldCommit = this->CommitIndex();
            if(this->commitIndex_ < leaderCommit)
            {
                this->commitIndex_ = (std::min)(leaderCommit,this->LastIndex());
            }
            while (begin != end)
            {
                if(begin->GetIndex() > oldCommit)
                {
                    this->lastApplied_ += 1;
                    this->Commiter().Commit(*begin);
                }
                ++begin;
            }
            return true;
        }

        void RaiseElection()
        {
            //reset leader
            this->ResetLeader();
            //set votes
            this->votes_ = 0;
            //increase term
            //and set role to candidate
            this->SetRole(sharpen::RaftRole::Candidate);
            this->PersistenceStorage().AddCurrentTerm();
            //vote to self
            this->PersistenceStorage().SetVotedFor(this->selfId_);
        }

        void ReactVote(sharpen::Uint64 vote)
        {
            this->votes_ += vote;
        }

        bool StopElection()
        {
            if (this->GetRole() == sharpen::RaftRole::Candidate && this->votes_ >= this->MemberMajority())
            {
                this->SetRole(sharpen::RaftRole::Leader);
                this->leaderId_.Construct(this->selfId_);
                return true;
            }
            return false;
        }

        bool RequestVote(sharpen::Uint64 candidateTerm,const _Id &candidateId,sharpen::Uint64 lastLogIndex,sharpen::Uint64 lastLogTerm)
        {
            //check term
            if(this->CurrentTerm() > candidateTerm)
            {
                return false;
            }
            //update term
            if(this->CurrentTerm() < candidateTerm)
            {
                this->BiggerTerm(candidateTerm);
                this->PersistenceStorage().ResetVotedFor();
                this->ResetLeader();
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
            //check term
            if (this->CurrentTerm() > leaderTerm)
            {
                return false;
            }
            if(this->CurrentTerm() < leaderTerm)
            {
                this->BiggerTerm(leaderTerm);
                this->leaderId_.Construct(leaderId);
            }
            else if(this->GetRole() == sharpen::RaftRole::Leader)
            {
                return false;
            }
            //access denied
            else if(this->KnowLeader())
            {
                if(this->LeaderId() != leaderId)
                {
                    return false;
                }
            }
            //new leader
            else
            {
                this->leaderId_.Construct(leaderId);
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
            return this->leaderId_.Get();
        }

        void ResetLeader()
        {
            this->leaderId_ = sharpen::NullOpt;
        }

        void PushLog(_Log log)
        {
            this->PersistenceStorage().PushLog(std::move(log));
        }

        void AddCommitIndex(sharpen::Uint64 value) noexcept
        {
            this->commitIndex_ += value;
        }

        void AddLastApplied(sharpen::Uint64 value) noexcept
        {
            this->lastApplied_ += value;
        }

        sharpen::Uint64 CommitIndex() const noexcept
        {
            return this->commitIndex_;
        }

        sharpen::Uint64 LastApplied() const noexcept
        {
            return this->lastApplied_;
        }

        void ApplyLogs()
        {
            while (this->lastApplied_ < this->commitIndex_)
            {
                if(this->lastApplied_ != 0)
                {
                    const _Log &log = this->PersistenceStorage().GetLog(this->lastApplied_);
                    this->Commiter().Commit(log);
                }
                ++this->lastApplied_;
            }
        }

        void CommitAndApplyLog(_Log log)
        {
            if(this->GetRole() == sharpen::RaftRole::Leader)
            {
                this->AddCommitIndex(1);
                this->Commiter().Commit(log);
                this->AddLastApplied(1);
            }
        }

        ~InternalRaftStateMachine() noexcept = default;
    };

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member>
    using RaftStateMachine = sharpen::InternalRaftStateMachine<_Id,_Log,_Commiter,_PersistenceStorage,_Member>;

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member>
    using RaftStateMachinePtr = std::shared_ptr<sharpen::RaftStateMachine<_Id,_Log,_Commiter,_PersistenceStorage,_Member>>;

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member,typename _StateMachine = sharpen::RaftStateMachine<_Id,_Log,_Commiter,_PersistenceStorage,_Member>,typename _Check = sharpen::IsCompletedType<_StateMachine>>
    std::shared_ptr<_StateMachine> MakeRaftStateMachine(_Id id,_PersistenceStorage pm)
    {
        return std::make_shared<_StateMachine>(std::move(id),std::move(pm));
    }

    template<typename _Id,typename  _Log,typename _Commiter,typename _PersistenceStorage,typename _Member,typename _StateMachine = sharpen::RaftStateMachine<_Id,_Log,_Commiter,_PersistenceStorage,_Member>,typename _Check = sharpen::IsCompletedType<_StateMachine>>
    std::shared_ptr<_StateMachine> MakeRaftStateMachine(_Id id,_PersistenceStorage pm,_Commiter commiter)
    {
        return std::make_shared<_StateMachine>(std::move(id),std::move(pm),std::move(commiter));
    }
}

#endif