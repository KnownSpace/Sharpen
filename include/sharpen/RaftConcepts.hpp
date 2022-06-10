#pragma once
#ifndef _SHARPEN_RAFTCONCEPTS_HPP
#define _SHARPEN_RAFTCONCEPTS_HPP

#include "TypeTraits.hpp"

namespace sharpen
{
    template<typename _Log>
    using InternalIsRaftLog = auto(*)()->decltype(std::declval<_Log>().GetTerm()
                                                    ,std::declval<_Log>().SetTerm(0)
                                                    ,std::declval<_Log>().GetIndex()
                                                    ,std::declval<_Log>().SetIndex(0));

    template<typename _Log>
    using IsRaftLog = sharpen::IsMatches<sharpen::InternalIsRaftLog,_Log>;

    template<typename _LogIterator>
    using IsRaftLogIterator = sharpen::IsRaftLog<decltype(*std::declval<_LogIterator>())>;

    template<typename _PersistentStorage,typename _Log,typename _Id>
    using InternalIsRaftPersistenceStorage = auto(*)()->decltype(std::declval<_PersistentStorage&>().AppendLog(std::declval<const _Log&>()/*log*/)
                                                                ,std::declval<_PersistentStorage&>().SetVotedFor(std::declval<const _Id&>()/*voteforId*/)
                                                                ,std::declval<_PersistentStorage&>().SetCurrentTerm(std::declval<std::uint64_t>()/*current term*/)
                                                                ,std::declval<_PersistentStorage&>().IncreaseCurrentTerm()
                                                                ,std::declval<std::uint64_t&>() = std::declval<const _PersistentStorage&>().GetCurrentTerm()
                                                                ,std::declval<_Id&>() = std::declval<const _PersistentStorage&>().GetVotedFor()
                                                                ,std::declval<bool&>() = std::declval<const _PersistentStorage&>().IsVotedFor()
                                                                ,std::declval<_PersistentStorage>().RemoveLogsAfter(std::declval<std::uint64_t>()/*begin log index*/)
                                                                ,std::declval<bool&>() = std::declval<const _PersistentStorage&>().ContainLog(std::declval<std::uint64_t>()/*log index*/)
                                                                ,std::declval<_Log&>() = std::declval<const _PersistentStorage&>().GetLog(std::declval<std::uint64_t>()/*log index*/)
                                                                ,std::declval<_Log&>() = std::declval<const _PersistentStorage&>().GetLastLog()
                                                                ,std::declval<std::uint64_t&>() = std::declval<const _PersistentStorage&>().GetLogsCount()
                                                                ,std::declval<bool&>() = std::declval<const _PersistentStorage&>().EmptyLogs()
                                                                ,std::declval<_PersistentStorage>().ResetVotedFor()
                                                                ,std::declval<bool&>() = std::declval<const _PersistentStorage&>().CheckLog(std::declval<std::uint64_t>()/*log index*/,std::declval<std::uint64_t>()/*expected log term*/)
                                                                ,std::declval<std::uint64_t&>() = std::declval<const _PersistentStorage&>().GetLastLogIndex()
                                                                ,std::declval<std::uint64_t&>() = std::declval<const _PersistentStorage&>().GetLastAppiledIndex()
                                                                ,std::declval<_PersistentStorage&>().SetLastAppiledIndex(static_cast<std::uint64_t>(0)));

    template<typename _PersistentStorage,typename _Log,typename _Id>
    using IsRaftPersistenceStorage = sharpen::IsMatches<sharpen::InternalIsRaftPersistenceStorage,_PersistentStorage,_Log,_Id>;

    template<typename _Id,typename _Member>
    using InternalIsRaftMember = auto(*)()->decltype(std::declval<_Id&>() = std::declval<const _Member&>().Id());
    
    template<typename _Id,typename _Member>
    using IsRaftMember = sharpen::IsMatches<sharpen::InternalIsRaftMember,_Id,_Member>;

    template<typename _Log,typename _Id,typename _Member,typename _PersistentStorage>
    struct InternalRaftApplicationHelper
    {
    private:
        using Self = InternalRaftApplicationHelper<_Log,_Id,_Member,_PersistentStorage>;

        template<typename _Application>
        static auto InternalApply(_Application &app,_Log log,std::unordered_map<_Id,_Member> &members,_PersistentStorage &storage,int,int,int) -> decltype(app.Apply(std::move(log),members,storage))
        {
            return app.Apply(std::move(log),members,storage);
        }

        template<typename _Application>
        static auto InternalApply(_Application &app,_Log log,std::unordered_map<_Id,_Member> &members,_PersistentStorage &storage,int,int,...) -> decltype(app.Apply(std::move(log),storage))
        {
            static_cast<void>(members);
            return app.Apply(std::move(log),storage);
        }

        template<typename _Application>
        static auto InternalApply(_Application &app,_Log log,std::unordered_map<_Id,_Member> &members,_PersistentStorage &storage,int,...) -> decltype(app.Apply(std::move(log),members))
        {
            static_cast<void>(storage);
            return app.Apply(std::move(log),members);
        }

        template<typename _Application>
        static auto InternalApply(_Application &app,_Log log,std::unordered_map<_Id,_Member> &members,_PersistentStorage &storage,...) -> decltype(app.Apply(std::move(log)))
        {
            static_cast<void>(members);
            static_cast<void>(storage);
            return app.Apply(std::move(log));
        }
    public:
        template<typename _Application>
        static auto Apply(_Application &app,_Log log,std::unordered_map<_Id,_Member> &members,_PersistentStorage &storage) -> decltype(Self::InternalApply(app,std::move(log),members,storage,0,0,0))
        {
            return Self::InternalApply(app,std::move(log),members,storage,0,0,0);
        }
    };

    template<typename _Log,typename _Id,typename _Member,typename _PersistentStorage,typename _Application>
    using InternalIsRaftApplication = auto(*)() -> decltype(sharpen::InternalRaftApplicationHelper<_Log,_Id,_Member,_PersistentStorage>::Apply(std::declval<_Application&>()/*app*/,std::declval<_Log>(),std::declval<std::unordered_map<_Id,_Member>&>() /*members*/,std::declval<_PersistentStorage&>() /*storage*/));

    template<typename _Log,typename _Id,typename _Member,typename _PersistentStorage,typename _Application>
    using IsRaftApplication = sharpen::IsMatches<sharpen::InternalIsRaftApplication,_Log,_Id,_Member,_PersistentStorage,_Application>;
}

#endif