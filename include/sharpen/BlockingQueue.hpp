#pragma once
#ifndef _SHARPEN_BLOCKINGQUEUE_HPP
#define _SHARPEN_BLOCKINGQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <list>
#include <memory>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "AsyncMutex.hpp"

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
        sharpen::AsyncMutex asyncLock_;
    public:
        BlockingQueue()
        :lock_()
        ,cond_()
        ,list_()
        ,asyncLock_()
        {}

        void Push(_T object) noexcept
        {
            {
                std::unique_lock lock(this->lock_);
                this->list_.push_back(std::move(object));
            }
            this->cond_.notice_once();
        }
        
        void PushAsync(_T object) noexcept
        {
            _T *p = new _T(std::move(object));
            assert(p != nullptr);
            this->asyncLock_.Lock([this,p](){
                std::unique_ptr<_T> obj(p);
                this->Push(std::move(*obj));
                this->asyncLock.Unlock();
            });
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
