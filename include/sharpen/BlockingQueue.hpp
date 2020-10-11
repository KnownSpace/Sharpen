#pragma once
#ifndef _SHARPEN_BLOCKINGQUEUE_HPP
#define _SHARPEN_BLOCKINGQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <list>
#include <type_trait>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

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
    public:
        void Push(_T object) noexcept
        {
            {
                std::unique_lock lock(this->lock_);
                this->list_.push_back(std::move(object));
            }
            this->cond_.notice_once();
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
