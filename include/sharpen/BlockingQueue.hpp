#pragma once
#ifndef _SHARPEN_BLOCKINGQUEUE_HPP
#define _SHARPEN_BLOCKINGQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <list>
#include <memory>
#include <thread>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    template<typename _T>
    class BlockingQueue:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using List = std::list<_T>;

        std::mutex lock_;
        std::condition_variable cond_;
        List list_;
        sharpen::SpinLock subLock_;
        bool subLocked_;
        List pendingList_;
        sharpen::Uint32 waiters_;
        
        bool LockSub() noexcept
        {
            bool state = true;
            std::swap(this->subLocked_,state);
            return !state;
        }
        
        List GetPending() noexcept
        {
            List pending;
            std::swap(pending,this->pendingList_);
            return pending;
        }
        
        void UnlockSub() noexcept
        {
            this->subLocked_ = false;
        }
    public:
        BlockingQueue()
        :lock_()
        ,cond_()
        ,list_()
        ,subLock_()
        ,subLocked_(false)
        ,pendingList_()
        ,waiters_(0)
        {}

        void Push(_T object)
        {
            //we push the object into pending list if cannot get the sub lock
            {
                std::unique_lock<sharpen::SpinLock> lock(this->subLock_);
                if(!this->LockSub())
                {
                    this->pendingList_.push_back(std::move(object));
                    return;
                }
            }
            //if we got the sub lock
            {
                    std::unique_lock<std::mutex> lock(this->lock_);
                    this->list_.push_back(std::move(object));
                    //get the lock again
                    std::unique_lock<sharpen::SpinLock> subLock(this->subLock_);
                    List &&pending = this->GetPending();
                    //handle pending list
                    for(auto begin = std::begin(pending),end = std::end(pending);begin != end;begin++)
                    {
                        this->list_.push_back(std::move(*begin));
                    }
                    this->UnlockSub();
                    if(this->waiters_ > 0)
                    {
                        //notify all threads
                        this->cond_.notify_all();
                    }
            }
        }
        
        _T Pop() noexcept
        {
            std::unique_lock<std::mutex> lock(this->lock_);
            sharpen::Uint16 tryCount = 0;
            while(this->list_.empty())
            {
                if(tryCount < 200)
                {
                    tryCount++;
                    std::this_thread::yield();
                    continue;
                }
                this->waiters_ += 1;
                this->cond_.wait(lock);
                this->waiters_ -= 1;
            }
            _T obj(std::move(this->list_.front()));
            this->list_.pop_front();
            return std::move(obj); 
        }
    };
}

#endif
