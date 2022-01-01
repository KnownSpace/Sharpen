#pragma once
#ifndef _SHARPEN_ASYNCBLOCKINGQUEUE_HPP
#define _SHARPEN_ASYNCBLOCKINGQUEUE_HPP

#include <list>

#include "AsyncSemaphore.hpp"

namespace sharpen
{
    template<typename _T>
    class AsyncBlockingQueue:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Storage = std::list<_T>;

        sharpen::SpinLock lock_;
        sharpen::AsyncSemaphore sign_;
        Storage list_;
    public:
        AsyncBlockingQueue()
            :sign_()
            ,lock_()
            ,list_()
        {}

        ~AsyncBlockingQueue() noexcept = default;

        _T Pop()
        {
            this->sign_.LockAsync();
            std::unique_lock<sharpen::SpinLock> lock(this->lock_);
            _T obj = std::move(this->list_.front());
            this->list_.pop_front();
            return obj;
        }

        void Push(_T obj)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                this->list_.push_back(std::move(obj));
            }
            this->sign_.Unlock();
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        void Emplace(_Args &&...args)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                this->list_.emplace(std::forward<_Args>(args)...);
            }
            this->sign_.Unlock();
        }
    }; 
}

#endif