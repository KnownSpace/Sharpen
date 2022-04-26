#pragma once
#ifndef _SHARPEN_RAFTWRAPPER_HPP
#define _SHARPEN_RAFTWRAPPER_HPP

#include <unordered_map>
#include <vector>
#include <iterator>
#include <memory>
#include <cassert>

#include "RaftRole.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "Future.hpp"
#include "CompressedPair.hpp"
#include "Optional.hpp"
#include "RaftConcepts.hpp"
#include "RaftApplyPolicy.hpp"

namespace sharpen
{

    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage>
    using RaftWrapperRequires = sharpen::BoolType<sharpen::IsRaftLog<_Log>::Value
                                                        && sharpen::IsRaftMember<_Id,_Member>::Value
                                                        && sharpen::IsRaftPersistenceStorage<_PersistentStorage,_Log,_Id>::Value
                                                        && sharpen::IsRaftApplication<_Log,_Id,_Member,_PersistentStorage,_Application>::Value>;

    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage,typename _Check = void>
    class InternalRaftWrapper;

    //0 is a sentinel value
    //it should not be used as log index
    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage>
    class InternalRaftWrapper<_Id,_Member,_Log,_Application,_PersistentStorage,sharpen::EnableIf<sharpen::RaftWrapperRequires<_Id,_Member,_Log,_Application,_PersistentStorage>::Value>>:public sharpen::Noncopyable
    {
    private:
        using MemberMap = std::unordered_map<_Id,_Member>;
        using Self = sharpen::InternalRaftWrapper<_Id,_Member,_Log,_Application,_PersistentStorage,sharpen::EnableIf<sharpen::RaftWrapperRequires<_Id,_Member,_Log,_Application,_PersistentStorage>::Value>>;

        _Id selfId_;
        sharpen::Optional<_Id> leaderId_;

        //persistence status
        //current term
        //logs[]
        //voted for
        _PersistentStorage storage_;

        //volatile status
        //current role
        sharpen::RaftRole role_;
        //commit index
        sharpen::Uint64 commitIndex_;
        //applied index
        sharpen::Uint64 lastApplied_;
        //application
        std::shared_ptr<_Application> application_;

        //leader voltile status
        //next index and match index
        MemberMap members_;

        //candidate voltile status
        sharpen::Uint64 votes_;

        void SetRole(sharpen::RaftRole role) noexcept
        {
            this->role_ = role;
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
            this->ResetLeader();
            this->SetCurrentTerm(term);
        }
    public:
        static constexpr sharpen::Uint64 sentinelLogIndex_{0};

        using LostPolicy = sharpen::RaftApplyPolicy;

        InternalRaftWrapper(_Id id,_PersistentStorage pm,std::shared_ptr<_Application> application)
            :selfId_(std::move(id))
            ,storage_(std::move(pm))
            ,role_(sharpen::RaftRole::Follower)
            ,commitIndex_(0)
            ,lastApplied_(0)
            ,application_(std::move(application))
            ,members_()
            ,votes_(0)
        {
            this->lastApplied_ = this->PersistenceStorage().GetLastAppiledIndex();
            this->commitIndex_ = this->lastApplied_;
        }

        InternalRaftWrapper(Self &&other) noexcept
            :selfId_(std::move(other.selfId_))
            ,storage_(std::move(other.storage_))
            ,role_(other.role_)
            ,commitIndex_(other.commitIndex_)
            ,lastApplied_(other.lastApplied_)
            ,application_(std::move(other.application_))
            ,members_(std::move(other.members_))
            ,votes_(other.votes_)
        {}

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->selfId_ = std::move(other.selfId_);
                this->storage_ = std::move(other.storage_);
                this->role_ = other.role_;
                this->commitIndex_ = other.commitIndex_;
                this->lastApplied_ = other.lastApplied_;
                this->application_ = std::move(other.application_);
                this->members_ = std::move(other.members_);
                this->votes_ = other.votes_;
            }
            return *this;
        }

        ~InternalRaftWrapper() noexcept = default;

        sharpen::Uint64 GetLastIndex() const noexcept
        {
            if(this->PersistenceStorage().EmptyLogs())
            {
                return 0;
            }
            return this->PersistenceStorage().GetLastLog().GetIndex();
        }

        sharpen::Uint64 GetLastTerm() const noexcept
        {
            if(this->PersistenceStorage().EmptyLogs())
            {
                return 0;
            }
            return this->PersistenceStorage().GetLastLog().GetTerm();
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
            return this->role_;
        }

        _Application &Application() noexcept
        {
            return *this->application_;
        }

        const _Application &Application() const noexcept
        {
            return *this->application_;
        }

        sharpen::Size MemberMajority() const noexcept
        {
            return (this->Members().size())/2;
        }

        void ReactNewTerm(sharpen::Uint64 term)
        {
            if(term > this->GetCurrentTerm())
            {
                this->BiggerTerm(term);
            }
        }

        const _PersistentStorage &PersistenceStorage() const noexcept
        {
            return this->storage_;
        }

        //used for snapshot
        //you should not remove logs than index bigger than last appiled
        //and never motify the contents of logs
        _PersistentStorage &PersistenceStorage() noexcept
        {
            return this->storage_;
        }

        const Self &Const() const noexcept
        {
            return *this;
        }

        //append entires
        template<typename _LogIterator,typename _Check = sharpen::EnableIf<sharpen::IsRaftLogIterator<_LogIterator>::Value>>
        bool AppendEntries(_LogIterator begin,_LogIterator end,const _Id &leaderId,sharpen::Uint64 leaderTerm,sharpen::Uint64 prevLogIndex,sharpen::Uint64 prevLogTerm,sharpen::Uint64 leaderCommit)
        {
            //if is old leader
            if(leaderTerm < this->GetCurrentTerm())
            {
                return false;
            }
            //update term
            if(this->GetCurrentTerm() < leaderTerm)
            {
                this->BiggerTerm(leaderTerm);
                this->leaderId_.Construct(leaderId);
            }
            else if(this->GetRole() == sharpen::RaftRole::Leader)
            {
                return false;
            }
            //access denied
            else if(this->KnowLeader() && this->GetLeaderId() != leaderId)
            {
                return false;
            }
            //new leader
            else
            {
                this->leaderId_.Construct(leaderId);
            }
            //reset voted for
            this->PersistenceStorage().ResetVotedFor();
            //check prev log
            if(prevLogIndex != Self::sentinelLogIndex_ && prevLogIndex > this->lastApplied_)
            {
                if(this->PersistenceStorage().ContainLog(prevLogIndex))
                {
                    if(!this->PersistenceStorage().CheckLog(prevLogIndex,prevLogTerm))
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            bool noskip{true};
            //erase logs if term diff with leader
            for (auto ite = begin; ite != end; ++ite)
            {
                //skip logs that already applied
                if(ite->GetIndex() > this->lastApplied_)
                {
                    bool containLog{noskip && this->PersistenceStorage().ContainLog(ite->GetIndex())};
                    if(containLog)
                    {
                        //check log
                        //if log term != leader
                        if(!this->PersistenceStorage().CheckLog(ite->GetIndex(),ite->GetTerm()))
                        {
                            //remove logs [index,end)
                            this->PersistenceStorage().RemoveLogsAfter(ite->GetIndex());
                            //skip check
                            //we already remove dirty logs
                            noskip = false;
                        }
                    }
                    if(!containLog)
                    {
                        this->PersistenceStorage().AppendLog(*ite);
                    }
                }
            }
            
            //set commit index
            if(this->commitIndex_ < leaderCommit)
            {
                //leader commit index may bigger than last index
                this->commitIndex_ = (std::min)(leaderCommit,this->GetLastIndex());
            }
            //apply logs
            //stop if we lost the logs
            this->ApplyLogs(LostPolicy::Stop);
            return true;
        }

        //we should wait a random time before you call it
        void RaiseElection()
        {
            //reset leader
            this->ResetLeader();
            //set votes
            this->votes_ = 0;
            //increase term
            //and set role to candidate
            this->SetRole(sharpen::RaftRole::Candidate);
            this->PersistenceStorage().IncreaseCurrentTerm();
            //vote to self
            this->PersistenceStorage().SetVotedFor(this->selfId_);
        }

        void ReactVote(sharpen::Uint64 vote)
        {
            this->votes_ += vote;
        }

        //should copy all logs to followers when become leader
        //and then apply logs
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
            //check role
            if(this->GetRole() == sharpen::RaftRole::Leader)
            {
                return false;
            }
            //check term
            if(this->GetCurrentTerm() > candidateTerm)
            {
                return false;
            }
            //update term
            if(this->GetCurrentTerm() < candidateTerm)
            {
                this->BiggerTerm(candidateTerm);
                this->PersistenceStorage().ResetVotedFor();
                this->ResetLeader();
            }
            if((!this->PersistenceStorage().IsVotedFor() || this->PersistenceStorage().GetVotedFor() == candidateId) && lastLogIndex >= this->GetLastIndex() && lastLogTerm >= this->GetLastTerm())
            {
                this->ConvertFollower();
                this->PersistenceStorage().SetVotedFor(candidateId);
                return true;
            }
            return false;
        }

        sharpen::Uint64 GetCurrentTerm() const noexcept
        {
            return this->PersistenceStorage().GetCurrentTerm();
        }

        const _Id &GetSelfId() const noexcept
        {
            return this->selfId_;
        }

        //return true if we continue
        //this impl is optional
        template<typename _UCommiter = _Application,typename _Check = decltype(std::declval<_UCommiter&>().InstallSnapshot(std::declval<_PersistentStorage&>(),std::declval<MemberMap&>(),0,0,0,0,nullptr,false)),typename _Assert = sharpen::EnableIf<std::is_same<_Application,_UCommiter>::value>>
        bool InstallSnapshot(const _Id &leaderId,sharpen::Uint64 leaderTerm,sharpen::Uint64 lastIncludedIndex,sharpen::Uint64 lastIncludedTerm,sharpen::Uint64 offset,const char *data,bool done)
        {
            //check term
            if (this->GetCurrentTerm() > leaderTerm)
            {
                return false;
            }
            if(this->GetCurrentTerm() < leaderTerm)
            {
                this->BiggerTerm(leaderTerm);
                this->leaderId_.Construct(leaderId);
            }
            else if(this->GetRole() == sharpen::RaftRole::Leader)
            {
                return false;
            }
            //access denied
            else if(this->KnowLeader() && this->GetLeaderId() != leaderId)
            {
                return false;
            }
            //new leader
            else
            {
                this->leaderId_.Construct(leaderId);
            }
            this->Application().InstallSnapshot(this->PersistenceStorage(),this->Members(),leaderTerm,lastIncludedIndex,lastIncludedTerm,offset,data,done);
            return !done;
        }

        bool KnowLeader() const noexcept
        {
            return this->leaderId_.Exist();
        }

        const _Id &GetLeaderId() const noexcept
        {
            return this->leaderId_.Get();
        }

        void ResetLeader()
        {
            this->leaderId_ = sharpen::EmptyOpt;
        }

        //for leader
        //you should append log
        //then add commit index
        //and append entires to followers
        //apply logs finally
        void AppendLog(_Log log)
        {
            this->PersistenceStorage().AppendLog(std::move(log));
        }

        void SetCommitIndex(sharpen::Uint64 index) noexcept
        {
            if(this->commitIndex_ < index)
            {
                this->commitIndex_ = index;
            }
        }

        sharpen::Uint64 GetCommitIndex() const noexcept
        {
            return this->commitIndex_;
        }

        sharpen::Uint64 GetLastApplied() const noexcept
        {
            return this->lastApplied_;
        }

        void ApplyLogs(LostPolicy policy)
        {
            while (this->lastApplied_ < this->commitIndex_)
            {
                ++this->lastApplied_;
                if(this->PersistenceStorage().ContainLog(this->lastApplied_))
                {
                    _Log log{this->PersistenceStorage().GetLog(this->lastApplied_)};
                    try
                    {
                        sharpen::InternalRaftApplicationHelper<_Log,_Id,_Member,_PersistentStorage>::Apply(this->Application(),std::move(log),this->Members(),this->PersistenceStorage());
                    }
                    catch(const std::exception&)
                    {
                        this->lastApplied_ -= 1;
                        throw;
                    }
                }
                //if we lost log and policy is stop
                else if(policy == LostPolicy::Stop)
                {
                    this->lastApplied_ -= 1;
                    break;
                }
                //optional operation
                this->PersistenceStorage().SetLastAppiledIndex(this->lastApplied_);
            }
        }

        static constexpr bool IsValidIndex(sharpen::Uint64 index) noexcept
        {
            return index == sentinelLogIndex_;
        }
    };

    template<typename _Id,typename _Member,typename  _Log,typename _Application,typename _PersistentStorage>
    using RaftWrapper = sharpen::InternalRaftWrapper<_Id,_Member,_Log,_Application,_PersistentStorage>;
}

#endif