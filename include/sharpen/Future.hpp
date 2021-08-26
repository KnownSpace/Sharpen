#pragma once
#ifndef _SHARPEN_FUTURE_HPP
#define _SHARPEN_FUTURE_HPP

#include <memory>
#include <condition_variable>
#include <functional>
#include <exception>
#include <stdexcept>

#include "SpinLock.hpp"
#include "TypeDef.hpp"

namespace sharpen
{

    enum class FutureState
    {
        Pending,
        Completed,
        Error
    };

    template<typename _Value>
    class Future:public sharpen::Noncopyable
    {
    private:
        using Self = Future<_Value>;
        using Callback = std::function<void(Self&)>;

        std::unique_ptr<sharpen::SpinLock> lock_;
        std::unique_ptr<_Value> value_;
        std::unique_ptr<std::condition_variable_any> cond_;
        Callback callback_;
        FutureState state_;
        std::exception_ptr error_;
        mutable sharpen::Uint16 waiters_;

        void NoticeIfNeed()
        {
            if (this->waiters_ != 0)
            {
                this->waiters_ = 0;
                this->cond_->notify_all();
            }
        }
    public:
        Future()
            :lock_(std::make_unique<sharpen::SpinLock>())
            ,value_(nullptr)
            ,cond_(std::make_unique<std::condition_variable_any>())
            ,callback_()
            ,state_(sharpen::FutureState::Pending)
            ,error_()
            ,waiters_(0)
        {
            if (!this->lock_ || !this->cond_)
            {
                throw std::bad_alloc();
            }
        }

        Future(Self &&other) noexcept
            :lock_(std::move(other.lock_))
            ,value_(std::move(other.value_))
            ,cond_(std::move(other.cond_))
            ,callback_(std::move(other.callback_))
            ,state_(other.state_)
            ,error_(std::move(other.error_))
            ,waiters_(other.waiters_)
        {}

        virtual ~Future() = default;

        Self &operator=(Self &&other) noexcept
        {
            this->lock_ = std::move(other.lock_);
            this->value_ = std::move(other.value_);
            this->cond_ = std::move(other.cond_);
            this->callback_ = std::move(other.callback_);
            this->state_ = std::move(other.state_);
            this->error_ = std::move(other.error_);
            this->waiters_ = other.waiters_;
            return *this;
        }

        void swap(Self &other) noexcept
        {
            this->lock_.swap(other.lock_);
            this->value_.swap(other.value_);
            this->cond_.swap(other.cond_);
            this->callback_.swap(other.callback_);
            std::swap(this->state_,other.state_);
            std::swap(this->error_,other.error_);
            std::swap(this->waiters_,other.waiters_);
        }

        inline void Swap(Self &&other) noexcept
        {
            this->swap(std::move(other));
        }

        template<typename ..._Args,typename = decltype(_Value(std::declval<_Args>()...))>
        void Complete(_Args &&...args)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                this->state_ = sharpen::FutureState::Completed;
                this->value_.reset(new _Value(args...));
            }
            this->ExecuteCallback();
        }

        void Fail(std::exception_ptr &&err)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                this->state_ = sharpen::FutureState::Error;
                this->error_ = std::move(err);
            }
            this->ExecuteCallback();
        }

        void Wait()
        {
            std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
            while (this->IsPending())
            {
                this->waiters_ += 1;
                this->cond_->wait(lock);
            }
        }

        _Value &Get()
        {
            this->Wait();
            if (this->state_ == sharpen::FutureState::Completed)
            {
                return *this->value_;
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        const _Value &Get() const
        {
            this->Wait();
            if (this->state_ == sharpen::FutureState::Completed)
            {
                return *this->value_;
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        bool CompletedOrError() const
        {
            return this->state_ != sharpen::FutureState::Pending;
        }

        bool IsPending() const
        {
            return this->state_ == sharpen::FutureState::Pending;
        }

        void SetCallback(Callback &&callback)
        {
            if (!callback)
            {
                return;
            }
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                if (this->IsPending())
                {
                    this->callback_ = std::move(callback);
                    return;
                }
            }
            callback(*this);
        }

        sharpen::SpinLock &GetCompleteLock()
        {
            return *this->lock_;
        }

        void Reset()
        {
            std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
            this->ResetWithoutLock();
        }
        
    protected:

        virtual void ResetWithoutLock()
        {
            this->value_.reset(nullptr);
            this->state_ = sharpen::FutureState::Pending;
        }

        virtual void ExecuteCallback()
        {
            this->NoticeIfNeed();
            if (this->callback_)
            {
                this->callback_(*this);
            }
        }
    };
    
    template<>
    class Future<void>
    {
    private:
        using Self = Future<void>;
        using Callback = std::function<void(Self&)>;

        std::unique_ptr<sharpen::SpinLock> lock_;
        std::unique_ptr<std::condition_variable_any> cond_;
        Callback callback_;
        FutureState state_;
        std::exception_ptr error_;
        mutable sharpen::Uint16 waiters_;

        void NoticeIfNeed()
        {
            if (this->waiters_ != 0)
            {
                this->waiters_ = 0;
                this->cond_->notify_all();
            }
        }

    public:

        Future()
            :lock_(std::make_unique<sharpen::SpinLock>())
            ,cond_(std::make_unique<std::condition_variable_any>())
            ,callback_()
            ,state_(sharpen::FutureState::Pending)
            ,error_()
            ,waiters_(0)
        {
            if (!this->lock_ || !this->cond_)
            {
                throw std::bad_alloc();
            }
        }

        Future(Self &&other) noexcept
            :lock_(std::move(other.lock_))
            ,cond_(std::move(other.cond_))
            ,callback_(std::move(other.callback_))
            ,state_(other.state_)
            ,error_(std::move(other.error_))
            ,waiters_(other.waiters_)
        {}

        virtual ~Future() = default;


        Self &operator=(Self &&other) noexcept
        {
            this->lock_ = std::move(other.lock_);
            this->cond_ = std::move(other.cond_);
            this->callback_ = std::move(other.callback_);
            this->state_ = std::move(other.state_);
            this->error_ = std::move(other.error_);
            this->waiters_ = other.waiters_;
            return *this;
        }

        void swap(Self &other) noexcept
        {
            this->lock_.swap(other.lock_);
            this->cond_.swap(other.cond_);
            this->callback_.swap(other.callback_);
            std::swap(this->state_,other.state_);
            std::swap(this->error_,other.error_);
            std::swap(this->waiters_,other.waiters_);
        }

        inline void Swap(Self &other) noexcept
        {
            this->swap(other);
        }

        void Complete()
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                this->state_ = sharpen::FutureState::Completed;
            }
            this->ExecuteCallback();
        }

        void Fail(std::exception_ptr &&err)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                this->state_ = sharpen::FutureState::Error;
                this->error_ = std::move(err);
            }
            this->ExecuteCallback();
        }

        void Wait()
        {
            std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
            while (this->IsPending())
            {
                this->waiters_ += 1;
                this->cond_->wait(lock);
            }
        }

        void Get()
        {
            this->Wait();
            if (this->state_ == sharpen::FutureState::Completed)
            {
                return;
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        bool CompletedOrError() const
        {
            return this->state_ != sharpen::FutureState::Pending;
        }

        bool IsPending() const
        {
            return this->state_ == sharpen::FutureState::Pending;
        }

        void SetCallback(Callback &&callback)
        {
            if (!callback)
            {
                return;
            }
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                if (this->IsPending())
                {
                    this->callback_ = std::move(callback);
                    return;
                }
            }
            callback(*this);
        }

        sharpen::SpinLock &GetCompleteLock()
        {
            return *this->lock_;
        }

        void Reset()
        {
            std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
            this->ResetWithoutLock();
        }
    protected:
        virtual void ResetWithoutLock()
        {
            this->state_ = sharpen::FutureState::Pending;
        }

        virtual void ExecuteCallback()
        {
            this->NoticeIfNeed();
            if (this->callback_)
            {
                this->callback_(*this);
            }
        }
    };

    template<typename _Value>
    using FuturePtr = std::shared_ptr<sharpen::Future<_Value>>;

    template<typename _Value>
    inline sharpen::FuturePtr<_Value> MakeFuturePtr()
    {
        auto p = std::make_shared<sharpen::Future<_Value>>();
        return std::move(p);
    }
} 

#endif
