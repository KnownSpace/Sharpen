#pragma once
#ifndef _SHARPEN_QUORUMCONCEPTS_HPP
#define _SHARPEN_QUORUMCONCEPTS_HPP

#include "TypeTraits.hpp"
#include "Future.hpp"

namespace sharpen
{
    template<typename _Proposer,typename _Proposal>
    using InternalIsQuorumProposer = auto(*)()->decltype(sharpen::DeclLvalue<_Proposer>().ProposeAsync(std::declval<_Proposal>(),std::declval<sharpen::Future<bool>&>()));

    template<typename _Proposer,typename _Proposal>
    using IsQuorumProposer = sharpen::IsMatches<sharpen::InternalIsQuorumProposer,_Proposer,_Proposal>;

    template<typename _ProposerIterator,typename _Proposal>
    using IsQuorumProposerIterator = sharpen::IsQuorumProposer<decltype(*std::declval<_ProposerIterator>()),_Proposal>;
}

#endif