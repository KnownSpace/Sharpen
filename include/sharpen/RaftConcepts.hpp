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

    template<typename _PersistenceStorage,typename _Log,typename _Id>
    using InternalIsRaftPersistenceStorage = auto(*)()->decltype(std::declval<_PersistenceStorage>().AppendLog(std::declval<const _Log&>()/*log*/)
                                                                ,std::declval<_PersistenceStorage>().SetVotedFor(std::declval<const _Id&>()/*voteforId*/)
                                                                ,std::declval<_PersistenceStorage>().SetCurrentTerm(std::declval<sharpen::Uint64>()/*current term*/)
                                                                ,std::declval<_PersistenceStorage>().AddCurrentTerm()
                                                                ,std::declval<sharpen::Uint64&>() = std::declval<const _PersistenceStorage&>().GetCurrentTerm()
                                                                ,std::declval<_Id&>() = std::declval<const _PersistenceStorage&>().GetVotedFor()
                                                                ,std::declval<bool&>() = std::declval<const _PersistenceStorage&>().IsVotedFor()
                                                                ,std::declval<_PersistenceStorage>().EraseLog(std::declval<sharpen::Uint64>()/*log index*/)
                                                                ,std::declval<bool&>() = std::declval<const _PersistenceStorage&>().ContainLog(std::declval<sharpen::Uint64>()/*log index*/)
                                                                ,std::declval<_Log&>() = std::declval<const _PersistenceStorage&>().GetLog(std::declval<sharpen::Uint64>()/*log index*/)
                                                                ,std::declval<_Log&>() = std::declval<const _PersistenceStorage&>().LastLog()
                                                                ,std::declval<sharpen::Size&>() = std::declval<const _PersistenceStorage&>().LogsCount()
                                                                ,std::declval<bool&>() = std::declval<const _PersistenceStorage&>().EmptyLogs()
                                                                ,std::declval<_PersistenceStorage>().ResetVotedFor()
                                                                ,std::declval<bool&>() = std::declval<const _PersistenceStorage&>().CheckLog(std::declval<sharpen::Uint64>()/*log index*/,std::declval<sharpen::Uint64>()/*expected log term*/)
                                                                ,std::declval<sharpen::Uint64&>() = std::declval<const _PersistenceStorage&>().LastLogIndex());

    template<typename _PersistenceStorage,typename _Log,typename _Id>
    using IsRaftPersistenceStorage = sharpen::IsMatches<sharpen::InternalIsRaftPersistenceStorage,_PersistenceStorage,_Log,_Id>;

    template<typename _Id,typename _Member>
    using InternalIsRaftMember = auto(*)()->decltype(std::declval<_Id&>() = std::declval<const _Member&>().Id()
                                                    ,std::declval<sharpen::Uint64&>() = std::declval<const _Member&>().GetNextIndex()
                                                    ,std::declval<sharpen::Uint64&>() = std::declval<const _Member&>().GetMatchIndex()
                                                    ,std::declval<_Member>().SetNextIndex(0)
                                                    ,std::declval<_Member>().SetMatchIndex(0));
    
    template<typename _Id,typename _Member>
    using IsRaftMember = sharpen::IsMatches<sharpen::InternalIsRaftMember,_Id,_Member>;

    template<typename _Log,typename _Application>
    using InternalIsRaftApplication = auto(*)() -> decltype(std::declval<_Application>().Commit(std::declval<_Log>()));

    template<typename _Log,typename _Application>
    using IsRaftApplication = sharpen::IsMatches<sharpen::InternalIsRaftApplication,_Log,_Application>;
}

#endif