#pragma once
#ifndef _SHARPEN_PROCESSQUEUE_HPP
#define _SHARPEN_PROCESSQUEUE_HPP

#include <vector>
#include <mutex>
#include <cassert>

#include "TypeDef.hpp"
#include "SpinLock.hpp"
#include "Future.hpp"
#include "AsyncMutex.hpp"

namespace sharpen
{
    template<typename _Item,typename _Result>
    class ProcessQueueItem
    {
    private:
        using Self = ProcessQueueItem<_Item,_Result>;
    
        _Item item_;
        sharpen::Future<_Result> *future_;
    public:
    
        ProcessQueueItem(_Item item,sharpen::Future<_Result> *future)
            :item_(std::move(item))
            ,future_(future)
        {
            assert(future != nullptr);
        }
    
        ProcessQueueItem(const Self &other) = default;
    
        ProcessQueueItem(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->item_ = std::move(other.item_);
                this->future_ = other.future_;
                other.future_ = nullptr;
            }
            return *this;
        }
    
        ~ProcessQueueItem() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::Future<_Result> *Future() noexcept
        {
            return this->future_;
        }

        _Item &Item() noexcept
        {
            return this->item_;
        }

        const _Item &Item() const noexcept
        {
            return this->item_;
        }
    };

    template<typename _Item,typename _Result>
    class ProcessQueue:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::ProcessQueue<_Item,_Result>;
        using Item = sharpen::ProcessQueueItem<_Item,_Result>;
        using Queue = std::vector<Item>;

        sharpen::SpinLock lock_;
        Queue queue_;
        Queue commitedQueue_;
        bool processing_;
        sharpen::AsyncMutex mutex_;
    protected:

        virtual void DoProcess(Queue &queue);
    public:
    
        ProcessQueue()
            :lock_()
            ,queue_()
            ,commitedQueue_()
            ,processing_(false)
            ,mutex_()
        {
            this->queue_.reserve(32);
            this->commitedQueue_.reserve(32);
        }
    
        ~ProcessQueue() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void ProcessAsync(_Item item,sharpen::Future<_Result> &future)
        {
            bool processing{true};
            {
                std::unique_lock<sharpen::SpinLock> lock{this->lock_};
                this->queue_.emplace_back(std::move(item),&future);
                std::swap(this->processing_,processing);
            }
            if(!processing)
            {
                {
                    std::unique_lock<sharpen::AsyncMutex> processLock{this->mutex_};
                    {
                        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
                        std::swap(this->queue_,this->commitedQueue_);
                        this->processing_ = false;
                    }
                    this->DoProcess(this->commitedQueue_);
                    this->commitedQueue_.clear();
                }
            }
        }

        inline _Result ProcessAsync(_Item item)
        {
            sharpen::AwaitableFuture<_Result> future;
            this->ProcessAsync(std::move(item),&future);
            return future.Await();
        }
    };
}

#endif