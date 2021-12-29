#pragma once
#ifndef _SHARPEN_QUORUMCONCEPTS_HPP
#define _SHARPEN_QUORUMCONCEPTS_HPP

#include "TypeTraits.hpp"
#include "Future.hpp"

namespace sharpen
{
    template<typename _Proposer,typename _Proposal>
    using InternalIsQuorumProposer = auto(*)()->decltype(sharpen::DeclLvalue<_Proposer>().ProposeAsync(std::declval<_Proposal>()/*proposal*/
                                                        ,std::declval<sharpen::Future<bool>&>())/*result future*/);

    template<typename _Proposer,typename _Proposal>
    using IsQuorumProposer = sharpen::IsMatches<sharpen::InternalIsQuorumProposer,_Proposer,_Proposal>;

    template<typename _ProposerIterator,typename _Proposal>
    using IsQuorumProposerIterator = sharpen::IsQuorumProposer<decltype(*std::declval<_ProposerIterator>()),_Proposal>;

    template<typename _ProposerMapIterator,typename _Proposal>
    using IsQuorumProposerMapIterator = sharpen::IsQuorumProposer<decltype(*std::declval<_ProposerMapIterator>()->second),_Proposal>;

    template<typename _Proposer>
    using InternalCancelableQuorumProposer = auto(*)() ->decltype(std::declval<_Proposer>().Cancel());

    template<typename _Proposer,typename _Proposal>
    using IsCancelableQuorumProposer = sharpen::BoolType<sharpen::IsQuorumProposer<_Proposer,_Proposal>::Value && sharpen::IsMatches<sharpen::InternalCancelableQuorumProposer,_Proposer>::Value>;

    template<typename _ProposerIterator,typename _Proposal>
    using IsCancelableQuorumProposerIterator = sharpen::IsCancelableQuorumProposer<decltype(*std::declval<_ProposerIterator>()),_Proposal>;

    template<typename _ProposerMapIterator,typename _Proposal>
    using IsCancelableQuorumProposerMapIterator = sharpen::IsCancelableQuorumProposer<decltype(*std::declval<_ProposerMapIterator>()->second),_Proposal>;
}

#endif