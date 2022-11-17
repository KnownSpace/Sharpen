#pragma once
#ifndef _SHARPEN_FUTURE_HPP
#define _SHARPEN_FUTURE_HPP

#include <memory>
#include <condition_variable>
#include <functional>
#include <exception>
#include <stdexcept>
#include <new>
#include <cstdint>
#include <cstddef>

#include "SpinLock.hpp"
#include "Optional.hpp"

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
        sharpen::Optional<_Value> value_;
        std::unique_ptr<std::condition_variable_any> cond_;
        Callback callback_;
        FutureState state_;
        std::exception_ptr error_;
        mutable std::uint16_t waiters_;

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
            :lock_(new (std::nothrow) sharpen::SpinLock())
            ,value_(sharpen::EmptyOpt)
            ,cond_(new (std::nothrow) std::condition_variable_any())
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

        Future(Self &&other) noexcept = default;

        virtual ~Future() = default;

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->value_ = std::move(other.value_);
                this->cond_ = std::move(other.cond_);
                this->callback_ = std::move(other.callback_);
                this->state_ = std::move(other.state_);
                this->error_ = std::move(other.error_);
                this->waiters_ = other.waiters_;
            }
            return *this;
        }

        template<typename ..._Args,typename = decltype(_Value{std::declval<_Args>()...})>
        void Complete(_Args &&...args)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                if(this->state_ != sharpen::FutureState::Pending)
                {
                    return;
                }
                this->state_ = sharpen::FutureState::Completed;
                this->value_.Construct(std::forward<_Args>(args)...);
            }
            this->ExecuteCallback();
        }

        inline void CompleteForBind(_Value val)
        {
            this->Complete(std::move(val));
        }

        void Fail(std::exception_ptr err)
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
                return this->value_.Get();
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        const _Value &Get() const
        {
            this->Wait();
            if (this->state_ == sharpen::FutureState::Completed)
            {
                return this->value_.Get();
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        bool CompletedOrError() const noexcept
        {
            return this->state_ != sharpen::FutureState::Pending;
        }

        bool IsPending() const noexcept
        {
            return this->state_ == sharpen::FutureState::Pending;
        }

        bool IsError() const noexcept
        {
            return this->state_ == sharpen::FutureState::Error;
        }

        bool IsCompleted() const noexcept
        {
            return this->state_ == sharpen::FutureState::Completed;
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

        std::exception_ptr Error() const
        {
            return this->error_;
        }
        
    protected:

        virtual void ResetWithoutLock()
        {
            this->value_.Reset();
            this->state_ = sharpen::FutureState::Pending;
        }

        virtual void ExecuteCallback()
        {
            this->NoticeIfNeed();
            Callback cb;
            std::swap(cb,this->callback_);
            if (cb)
            {
                cb(*this);
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
        mutable std::uint16_t waiters_;

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
            :lock_(new (std::nothrow) sharpen::SpinLock())
            ,cond_(new (std::nothrow) std::condition_variable_any())
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

        Future(Self &&other) noexcept = default;

        virtual ~Future() = default;


        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->cond_ = std::move(other.cond_);
                this->callback_ = std::move(other.callback_);
                this->state_ = std::move(other.state_);
                this->error_ = std::move(other.error_);
                this->waiters_ = other.waiters_;
            }
            return *this;
        }

        void Complete()
        {
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                if(this->state_ != sharpen::FutureState::Pending)
                {
                    return;
                }
                this->state_ = sharpen::FutureState::Completed;
            }
            this->ExecuteCallback();
        }

        inline void CompleteForBind()
        {
            this->Complete();
        }

        void Fail(std::exception_ptr err)
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

        bool CompletedOrError() const noexcept
        {
            return this->state_ != sharpen::FutureState::Pending;
        }

        bool IsPending() const noexcept
        {
            return this->state_ == sharpen::FutureState::Pending;
        }

        bool IsError() const noexcept
        {
            return this->state_ == sharpen::FutureState::Error;
        }

        bool IsCompleted() const noexcept
        {
            return this->state_ == sharpen::FutureState::Completed;
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

        std::exception_ptr Error() const
        {
            return this->error_;
        }
    protected:
        virtual void ResetWithoutLock()
        {
            this->state_ = sharpen::FutureState::Pending;
        }

        virtual void ExecuteCallback()
        {
            this->NoticeIfNeed();
            Callback cb;
            std::swap(cb,this->callback_);
            if (cb)
            {
                cb(*this);
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
