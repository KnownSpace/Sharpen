#pragma once
#ifndef _SHARPEN_QUORUM_HPP
#define _SHARPEN_QUORUM_HPP

#include <cassert>
#include <memory>
#include <atomic>
#include <functional>
#include <vector>

#include "TypeDef.hpp"
#include "Future.hpp"
#include "IteratorOps.hpp"
#include "QuorumConcepts.hpp"

namespace sharpen
{
    template<typename _Prposal,typename _Proposer,typename _Check = void>
    class InternalQuorum;

    template<typename _Proposal,typename _Proposer>
    class InternalQuorum<_Proposal,_Proposer,sharpen::EnableIf<sharpen::IsQuorumProposer<_Proposer,_Proposal>::Value>>:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::InternalQuorum<_Proposal,_Proposer,sharpen::EnableIf<sharpen::IsQuorumProposer<_Proposer,_Proposal>::Value>>;

        struct Waiter
        {
            std::atomic_size_t finishCounter_;
            std::atomic_size_t successCounter_;
            sharpen::Future<bool>* continuation_;
            sharpen::Future<void>* finish_;
            sharpen::Size majority_;
            std::vector<sharpen::Future<bool>> futures_;
        };

        static void CompletedCallback(sharpen::Future<bool> &future,std::shared_ptr<Waiter> waiterPtr)
        {
            bool success = !future.IsError() && future.Get();
            sharpen::Size finishCounter = waiterPtr->finishCounter_.fetch_sub(1) - 1;
            if (success)
            {
                sharpen::Size successCounter = waiterPtr->successCounter_.fetch_add(1) + 1;
                if(successCounter == waiterPtr->majority_)
                {
                    sharpen::Future<bool> *continuation{nullptr};
                    std::swap(continuation,waiterPtr->continuation_);
                    if(continuation)
                    {
                        continuation->Complete(true);
                    }
                }
            }
            if(finishCounter == 0)
            {
                waiterPtr->finish_->Complete();
                sharpen::Future<bool> *continuation{nullptr};
                std::swap(continuation,waiterPtr->continuation_);
                if(continuation)
                {
                    continuation->Complete(false);
                }
            }
        }

    public:
        template<typename _Iterator>
        void ProposeAsync(_Iterator begin,_Iterator end,const _Proposal &proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> &finish)
        {
            using FnPtr = void(*)(sharpen::Future<bool>&,std::shared_ptr<Waiter>);
            //init waiter
            std::shared_ptr<Waiter> waiterPtr = std::make_shared<Waiter>();
            sharpen::Size size = sharpen::GetRangeSize(begin,end);
            waiterPtr->finish_ = &finish;
            waiterPtr->continuation_ = &continuation;
            waiterPtr->finishCounter_.store(size);
            waiterPtr->successCounter_.store(0);
            waiterPtr->majority_ = (size + 1)/2;
            waiterPtr->futures_.resize(size);
            sharpen::Size index{0};
            //launch operations
            while (begin != end)
            {
                waiterPtr->futures_[index].SetCallback(std::bind(static_cast<FnPtr>(&Self::CompletedCallback),std::placeholders::_1,waiterPtr));
                begin->ProposeAsync(proposal,waiterPtr->futures_[index]);
                ++index;
                ++begin;
            }
        }
    };

    template<typename _Proposal,typename _Proposer>
    using Quorum = sharpen::InternalQuorum<_Proposal,_Proposer>;
}

#endif