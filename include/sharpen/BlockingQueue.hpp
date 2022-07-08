#pragma once
#ifndef _SHARPEN_BLOCKINGQUEUE_HPP
#define _SHARPEN_BLOCKINGQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <thread>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"
#include <cstdint>
#include <cstddef>

namespace sharpen
{
    //LIFO
    //reduce the number of memory allocations
    template<typename _T>
    class BlockingQueue:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Storage = std::vector<_T>;
        using Lock = std::mutex;
        using CondVar = std::condition_variable;

        Lock lock_;
        CondVar cond_;
        Storage storage_;
        
    public:
        BlockingQueue() = default;

        ~BlockingQueue() noexcept = default;

        void Push(_T object)
        {
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->storage_.push_back(std::move(object));
            }
            this->cond_.notify_one();
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        void Emplace(_Args &&...args)
        {
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->storage_.emplace_back(std::forward<_Args>(args)...);
            }
            this->cond_.notify_one();
        }
        
        _T Pop() noexcept
        {
            std::unique_lock<Lock> lock(this->lock_);
            while(this->storage_.empty())
            {
                this->cond_.wait(lock);
            }
            _T obj(std::move(this->storage_.back()));
            this->storage_.pop_back();
            return std::move(obj); 
        }

        template <class _Rep, class _Period>
        bool Pop(_T &obj,const std::chrono::duration<_Rep, _Period> &timeout) noexcept
        {
            std::unique_lock<Lock> lock(this->lock_);
            while(this->storage_.empty())
            {
                std::cv_status status = this->cond_.wait_for(lock,timeout);
                if (status == std::cv_status::timeout)
                {
                    return false;
                }
            }
            obj = std::move(this->storage_.back());
            this->storage_.pop_back();
            return true;
        }
    };
}

#endif
