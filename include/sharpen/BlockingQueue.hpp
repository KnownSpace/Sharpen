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
        using Storage = std::list<_T>;
        using Lock = std::mutex;
        using CondVar = std::condition_variable;

        Lock lock_;
        CondVar cond_;
        Storage list_;
        
    public:
        BlockingQueue()
        :lock_()
        ,cond_()
        ,list_()
        {}

        void Push(_T object)
        {
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->list_.push_back(std::move(object));
            }
            this->cond_.notify_one();
        }
        
        _T Pop() noexcept
        {
            std::unique_lock<Lock> lock(this->lock_);
            while(this->list_.empty())
            {
                this->cond_.wait(lock);
            }
            _T obj(std::move(this->list_.front()));
            this->list_.pop_front();
            return std::move(obj); 
        }

        template <class _Rep, class _Period>
        bool Pop(_T &obj,const std::chrono::duration<_Rep, _Period> &timeout) noexcept
        {
            std::unique_lock<Lock> lock(this->lock_);
            while(this->list_.empty())
            {
                std::cv_status status = this->cond_.wait_for(lock,timeout);
                if (status == std::cv_status::timeout)
                {
                    return false;
                }
            }
            obj = std::move(this->list_.front());
            this->list_.pop_front();
            return true;
        }
    };
}

#endif
