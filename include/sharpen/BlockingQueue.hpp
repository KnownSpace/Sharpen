#pragma once
#ifndef _SHARPEN_BLOCKINGQUEUE_HPP
#define _SHARPEN_BLOCKINGQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <list>
#include <memory>
#include <cassert>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"

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
        
        bool LockSub()
        {
            bool state = true;
            std::swap(this->subLocked_,state);
            return !state;
        }
        
        List GetPending()
        {
            List pending;
            std::swap(pending,this->pendingList_);
            return pending;
        }
        
        void UnlockSub()
        {
            this->subLocked_ = false;
        }
    public:
        BlockingQueue()
        :lock_()
        ,cond_()
        ,list_()
        ,subLock_()
        ,subLocked(false)
        ,pendingList_()
        {}

        void Push(_T object) noexcept
        {
            //we push the object into pending list if cannot get the sub lock
            bool getSubLock(false);
            {
                std::unique_lock<sharpen::SpinLock> lock(this->subLock_);
                getSubLock = this->LockSub();
                if(!getSubLock)
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
                    List &&pending = GetPending();
                    //handle pending list
                    for(auto begin = std::begin(pending),end = std::end(pending);begin != end;begin++)
                    {
                        this->list_.push_back(std::move(*begin));
                    }
                    this->UnlockSub();
            }
            //notice all threads
            this->cond_.notice_all();
        }
        
        _T Pop() noexcept
        {
            std::unique_lock lock(this->lock_);
            while(this->list_.empty())
            {
                this->cond_.wait(lock);
            }
            _T obj(std::move(this->list_.front()));
            this->list_.pop_front();
            return std::move(obj);
        }
    };
}

#endif
