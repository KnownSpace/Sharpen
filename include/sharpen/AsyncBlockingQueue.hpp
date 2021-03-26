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
            return std::move(obj);
        }

        void Push(_T obj)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                this->list_.push_back(std::move(obj));
            }
            this->sign_.Unlock();
        }
    }; 
}

#endif