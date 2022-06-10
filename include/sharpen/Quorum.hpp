#pragma once
#ifndef _SHARPEN_QUORUM_HPP
#define _SHARPEN_QUORUM_HPP

#include <cassert>
#include <memory>
#include <atomic>
#include <functional>
#include <vector>
#include <chrono>

#include <cstdint>
#include <cstddef>
#include "Future.hpp"
#include "IteratorOps.hpp"
#include "QuorumConcepts.hpp"
#include "ITimer.hpp"

namespace sharpen
{
    class Quorum
    {
    private:
        using Self = sharpen::Quorum;

        struct Waiter
        {
            std::atomic_size_t finishCounter_;
            std::atomic_size_t successCounter_;
            std::atomic_size_t errorCounter_;
            sharpen::Future<bool>* continuation_;
            sharpen::Future<void>* finish_;
            std::size_t majority_;
            std::vector<sharpen::Future<bool>> futures_;
        };

        template<typename _ProposerIterator>
        struct TimeLimitedWaiter:public Waiter
        {
            _ProposerIterator iterator_;
            sharpen::Future<bool> timeoutFuture_;
            sharpen::TimerPtr timer_;
        };

        template<typename _T,typename ..._Args>
        static auto CompleteFuture(sharpen::Future<_T>* &future,_Args &&...args) -> decltype(future->Complete(std::forward<_Args>(args)...))
        {
            sharpen::Future<_T> *f{nullptr};
            std::swap(f,future);
            if(f)
            {
                f->Complete(std::forward<_Args>(args)...);
            }
        }

        static void CompletedCallback(sharpen::Future<bool> &future,std::shared_ptr<Waiter> waiterPtr)
        {
            bool success = !future.IsError() && future.Get();
            std::size_t finishCounter = waiterPtr->finishCounter_.fetch_sub(1) - 1;
            if (success)
            {
                std::size_t successCounter = waiterPtr->successCounter_.fetch_add(1) + 1;
                if(successCounter == waiterPtr->majority_)
                {
                    CompleteFuture(waiterPtr->continuation_,true);
                }
            }
            else
            {
                std::size_t errorCount = waiterPtr->errorCounter_.fetch_add(1);
                if(waiterPtr->futures_.size() - errorCount == waiterPtr->majority_)
                {
                    CompleteFuture(waiterPtr->continuation_,false);
                }
            }
            if(finishCounter == 0)
            {
                CompleteFuture(waiterPtr->finish_);
            }
        }

        template<typename _Iterator>
        struct TimeLimitedCompletedCallback
        {
            std::shared_ptr<TimeLimitedWaiter<_Iterator>> waiterPtr_;

            void operator()(sharpen::Future<bool> &future)
            {
                bool success = !future.IsError() && future.Get();
                std::size_t finishCounter = this->waiterPtr_->finishCounter_.fetch_sub(1) - 1;
                if (success)
                {
                    std::size_t successCounter = this->waiterPtr_->successCounter_.fetch_add(1) + 1;
                    if(successCounter == this->waiterPtr_->majority_)
                    {
                        CompleteFuture(this->waiterPtr_->continuation_,true);
                    }
                }
                else
                {
                    std::size_t errorCount = this->waiterPtr_->errorCounter_.fetch_add(1);
                    if(this->waiterPtr_->futures_.size() - errorCount == this->waiterPtr_->majority_)
                    {
                        CompleteFuture(this->waiterPtr_->continuation_,false);
                    }
                }
                if(finishCounter == 0)
                {
                    this->waiterPtr_->timer_->Cancel();
                    CompleteFuture(this->waiterPtr_->finish_);
                }
            }
        };

        template<typename _Iterator>
        struct IteratorTimeoutCallback
        {
            std::shared_ptr<TimeLimitedWaiter<_Iterator>> waiter_;

            void operator()(sharpen::Future<bool> &future)
            {
                if(future.Get())
                {
                    _Iterator begin = this->waiter_->iterator_;
                    _Iterator end = sharpen::IteratorForward(begin,this->waiter_->futures_.size());
                    _Iterator ite = end;
                    for (std::size_t i = 0; i != this->waiter_->futures_.size(); ++i)
                    {
                        if(this->waiter_->futures_[i].IsPending())
                        {
                            _Iterator copy = begin;
                            std::swap(copy,ite);
                            if(copy != end)
                            {
                                copy->Cancel();
                            }
                        }
                        ++begin;
                    }
                    if(ite != end)
                    {
                        ite->Cancel();
                    }
                }
            }
        };

        template<typename _Iterator>
        struct MapIteratorTimeoutCallback
        {
            std::shared_ptr<TimeLimitedWaiter<_Iterator>> waiter_;

            void operator()(sharpen::Future<bool> &future)
            {
                if(future.Get())
                {
                    _Iterator begin = waiter_->iterator_;
                    _Iterator end = sharpen::IteratorForward(begin,this->waiter_->futures_.size());
                    _Iterator ite = end;
                    for (std::size_t i = 0; i < waiter_->futures_.size(); i++)
                    {
                        if(waiter_->futures_[i].IsPending())
                        {
                            _Iterator copy = begin;
                            std::swap(copy,ite);
                            if(copy != end)
                            {
                                copy->second.Cancel();
                            }
                        }
                        ++begin;
                    }
                    if(ite != end)
                    {
                        ite->second.Cancel();
                    }
                }
            }
        };

        static void InitWaiter(std::shared_ptr<Waiter> &waiterPtr,std::size_t size,sharpen::Future<bool> *continuation,sharpen::Future<void> *finish,std::size_t majority) noexcept
        {
            waiterPtr->finish_ = finish;
            waiterPtr->continuation_ = continuation;
            waiterPtr->finishCounter_.store(size);
            waiterPtr->successCounter_.store(0);
            waiterPtr->majority_ = majority;
            waiterPtr->futures_.resize(size);
            waiterPtr->errorCounter_.store(0);
        }

        template<typename _Iterator>
        static void InitTimeLimitedWaiter(std::shared_ptr<TimeLimitedWaiter<_Iterator>> &waiterPtr,sharpen::TimerPtr timer,_Iterator iterator,std::size_t size,sharpen::Future<bool> *continuation,sharpen::Future<void> *finish,std::size_t majority) noexcept
        {
            waiterPtr->finish_ = finish;
            waiterPtr->continuation_ = continuation;
            waiterPtr->finishCounter_.store(size);
            waiterPtr->successCounter_.store(0);
            waiterPtr->majority_ = majority;
            waiterPtr->futures_.resize(size);
            waiterPtr->errorCounter_.store(0);
            waiterPtr->iterator_ = iterator;
            waiterPtr->timer_ = std::move(timer);
        }

        static constexpr bool emptyQuorumResult_{true};

        template<typename _Iterator,typename _Proposal,typename _Check = sharpen::EnableIf<sharpen::IsQuorumProposerIterator<_Iterator,_Proposal>::Value>>
        static void InternalProposeAsync(_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> *finish,std::size_t majority,...)
        {
            using FnPtr = void(*)(sharpen::Future<bool>&,std::shared_ptr<Waiter>);
            //init waiter
            std::shared_ptr<Waiter> waiterPtr = std::make_shared<Waiter>();
            std::size_t size = sharpen::GetRangeSize(begin,end);
            if(!size)
            {
                continuation.CompleteForBind(Self::emptyQuorumResult_);
                finish->Complete();
                return;
            }
            assert(majority != 0);
            InitWaiter(waiterPtr,size,&continuation,finish,majority);
            std::size_t index{0};
            //launch operations
            while (begin != end)
            {
                waiterPtr->futures_[index].SetCallback(std::bind(static_cast<FnPtr>(&Self::CompletedCallback),std::placeholders::_1,waiterPtr));
                begin->ProposeAsync(std::forward<_Proposal>(proposal),waiterPtr->futures_[index]);
                ++index;
                ++begin;
            }
        }

        template<typename _Iterator,typename _Proposal,typename _Check = sharpen::EnableIf<sharpen::IsQuorumProposerMapIterator<_Iterator,_Proposal>::Value>>
        static void InternalProposeAsync(_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> *finish,std::size_t majority,int)
        {
            using FnPtr = void(*)(sharpen::Future<bool>&,std::shared_ptr<Waiter>);
            //init waiter
            std::shared_ptr<Waiter> waiterPtr = std::make_shared<Waiter>();
            std::size_t size = sharpen::GetRangeSize(begin,end);
            if(!size)
            {
                continuation.CompleteForBind(Self::emptyQuorumResult_);
                finish->Complete();
                return;
            }
            assert(majority != 0);
            InitWaiter(waiterPtr,size,&continuation,finish,majority);
            std::size_t index{0};
            //launch operations
            while (begin != end)
            {
                waiterPtr->futures_[index].SetCallback(std::bind(static_cast<FnPtr>(&Self::CompletedCallback),std::placeholders::_1,waiterPtr));
                begin->second.ProposeAsync(std::forward<_Proposal>(proposal),waiterPtr->futures_[index]);
                ++index;
                ++begin;
            }
        }

        template<typename _Iterator,typename _Proposal,typename _Rep,typename _Period,typename _Check = sharpen::EnableIf<sharpen::IsCancelableQuorumProposerIterator<_Iterator,_Proposal>::Value>>
        static void InternalTimeLimitedProposeAsync(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> *finish,std::size_t majority,...)
        {
            using WaiterType = TimeLimitedWaiter<_Iterator>;
            //init waiter
            std::shared_ptr<WaiterType> waiterPtr = std::make_shared<WaiterType>();
            std::size_t size = sharpen::GetRangeSize(begin,end);
            if(!size)
            {
                continuation.CompleteForBind(Self::emptyQuorumResult_);
                finish->Complete();
                return;
            }
            assert(majority != 0);
            InitTimeLimitedWaiter(waiterPtr,std::move(timer),begin,size,&continuation,finish,majority);
            std::size_t index{0};
            //launch operations
            IteratorTimeoutCallback<_Iterator> timeoutCb;
            timeoutCb.waiter_ = waiterPtr;
            waiterPtr->timeoutFuture_.SetCallback(std::move(timeoutCb));
            waiterPtr->timer_->WaitAsync(waiterPtr->timeoutFuture_,timeout);
            while (begin != end)
            {
                TimeLimitedCompletedCallback<_Iterator> cb;
                cb.waiterPtr_ = waiterPtr;
                waiterPtr->futures_[index].SetCallback(std::move(cb));
                begin->ProposeAsync(std::forward<_Proposal>(proposal),waiterPtr->futures_[index]);
                ++index;
                ++begin;
            }
        }

        template<typename _MapIterator,typename _Proposal,typename _Rep,typename _Period,typename _Check = sharpen::EnableIf<sharpen::IsCancelableQuorumProposerMapIterator<_MapIterator,_Proposal>::Value>>
        static void InternalTimeLimitedProposeAsync(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,_MapIterator begin,_MapIterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> *finish,std::size_t majority,int)
        {
            using WaiterType = TimeLimitedWaiter<_MapIterator>;
            //init waiter
            std::shared_ptr<WaiterType> waiterPtr = std::make_shared<WaiterType>();
            std::size_t size = sharpen::GetRangeSize(begin,end);
            if(!size)
            {
                continuation.CompleteForBind(Self::emptyQuorumResult_);
                finish->Complete();
                return;
            }
            assert(majority != 0);
            InitTimeLimitedWaiter(waiterPtr,std::move(timer),begin,size,&continuation,finish,majority);
            std::size_t index{0};
            //launch operations
            MapIteratorTimeoutCallback<_MapIterator> timeoutCb;
            timeoutCb.waiter_ = waiterPtr;
            waiterPtr->timeoutFuture_.SetCallback(std::move(timeoutCb));
            waiterPtr->timer_->WaitAsync(waiterPtr->timeoutFuture_,timeout);
            while (begin != end)
            {
                TimeLimitedCompletedCallback<_MapIterator> cb;
                cb.waiterPtr_ = waiterPtr;
                waiterPtr->futures_[index].SetCallback(std::move(cb));
                begin->second.ProposeAsync(std::forward<_Proposal>(proposal),waiterPtr->futures_[index]);
                ++index;
                ++begin;
            }
        }

    public:
        template<typename _Iterator,typename _Proposal>
        static auto ProposeAsync(_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> &finish) ->decltype(Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,&finish,0,0))
        {
            return Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,&finish,(sharpen::GetRangeSize(begin,end) + 1)/2,0);
        }

        template<typename _Iterator,typename _Proposal>
        static auto ProposeAsync(_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation) ->decltype(Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,0,0))
        {
            return Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,(sharpen::GetRangeSize(begin,end) + 1)/2,0);
        }

        template<typename _Iterator,typename _Proposal>
        static auto ProposeAsync(_Iterator begin,_Iterator end,std::size_t majority,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> &finish) ->decltype(Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,&finish,0,0))
        {
            return Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,&finish,majority,0);
        }

        template<typename _Iterator,typename _Proposal>
        static auto ProposeAsync(_Iterator begin,_Iterator end,std::size_t majority,_Proposal &&proposal,sharpen::Future<bool> &continuation) ->decltype(Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,0,0))
        {
            return Self::InternalProposeAsync(begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,majority,0);
        }

        template<typename _Iterator,typename _Proposal,typename _Rep,typename _Period>
        static auto TimeLimitedProposeAsync(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> &finish) ->decltype(Self::InternalTimeLimitedProposeAsync(timer,timeout,begin,end,proposal,continuation,&finish,0,0))
        {
            return Self::InternalTimeLimitedProposeAsync(std::move(timer),timeout,begin,end,std::forward<_Proposal>(proposal),continuation,&finish,(sharpen::GetRangeSize(begin,end) + 1)/2,0);
        }

        template<typename _Iterator,typename _Proposal,typename _Rep,typename _Period>
        static auto TimeLimitedProposeAsync(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,_Iterator begin,_Iterator end,_Proposal &&proposal,sharpen::Future<bool> &continuation) ->decltype(Self::InternalTimeLimitedProposeAsync(timer,timeout,begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,0,0))
        {
            return Self::InternalTimeLimitedProposeAsync(std::move(timer),timeout,begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,(sharpen::GetRangeSize(begin,end) + 1)/2,0);
        }

        template<typename _Iterator,typename _Proposal,typename _Rep,typename _Period>
        static auto TimeLimitedProposeAsync(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,_Iterator begin,_Iterator end,std::size_t majority,_Proposal &&proposal,sharpen::Future<bool> &continuation,sharpen::Future<void> &finish) ->decltype(Self::InternalTimeLimitedProposeAsync(timer,timeout,begin,end,std::forward<_Proposal>(proposal),continuation,&finish,0,0))
        {
            return Self::InternalTimeLimitedProposeAsync(std::move(timer),timeout,begin,end,std::forward<_Proposal>(proposal),continuation,&finish,majority,0);
        }

        template<typename _Iterator,typename _Proposal,typename _Rep,typename _Period>
        static auto TimeLimitedProposeAsync(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &timeout,_Iterator begin,_Iterator end,std::size_t majority,_Proposal &&proposal,sharpen::Future<bool> &continuation) ->decltype(Self::InternalTimeLimitedProposeAsync(timer,timeout,begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,0,0))
        {
            return Self::InternalTimeLimitedProposeAsync(std::move(timer),timeout,begin,end,std::forward<_Proposal>(proposal),continuation,nullptr,majority,0);
        }
    };
}

#endif