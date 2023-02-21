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
    protected:
        using Callback = std::function<void(Self &)>;
    private:
        std::unique_ptr<sharpen::SpinLock> lock_;
        sharpen::Optional<_Value> value_;
        Callback callback_;
        std::atomic<FutureState> state_;
        std::exception_ptr error_;
    public:
        Future()
            :lock_(new (std::nothrow) sharpen::SpinLock())
            ,value_(sharpen::EmptyOpt)
            ,callback_()
            ,state_(sharpen::FutureState::Pending)
            ,error_()
        {
            if(!this->lock_)
            {
                throw std::bad_alloc();
            }
        }

        Future(Self &&other) noexcept
            :lock_(std::move(other.lock_))
            ,value_(std::move(other.value_))
            ,callback_(std::move(other.callback_))
            ,state_(other.state_.load(std::memory_order::memory_order_relaxed))
            ,error_(std::move(other.error_))
        {
            other.state_.store(sharpen::FutureState::Pending,std::memory_order::memory_order_relaxed);
        }

        virtual ~Future() = default;

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->value_ = std::move(other.value_);
                this->callback_ = std::move(other.callback_);
                // this->state_ = std::move(other.state_);
                sharpen::FutureState state{other.state_.load(std::memory_order::memory_order_relaxed)};
                this->state_.store(state,std::memory_order::memory_order_relaxed);
                this->error_ = std::move(other.error_);
                other.state_.store(sharpen::FutureState::Pending,std::memory_order::memory_order_relaxed);
            }
            return *this;
        }

        template<typename ..._Args,typename = decltype(_Value{std::declval<_Args>()...})>
        void Complete(_Args &&...args)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                this->value_.Construct(std::forward<_Args>(args)...);
            }
            this->ExecuteCallback(sharpen::FutureState::Completed);
        }

        inline void CompleteForBind(_Value val)
        {
            this->Complete(std::move(val));
        }

        inline void Fail(std::exception_ptr err)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                this->error_ = std::move(err);
            }
            this->ExecuteCallback(sharpen::FutureState::Error);
        }

        void Wait() const
        {
            // std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
            // while(this->IsPending() && !this->value_.Exist() && !this->error_)
            while(this->IsPending())
            {
                // lock.unlock();
                std::this_thread::yield();
                // lock.lock();
            }
        }

        inline _Value &Get()
        {
            this->Wait();
            if(this->state_.load(std::memory_order::memory_order_relaxed) == sharpen::FutureState::Completed)
            {
                return this->value_.Get();
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        inline const _Value &Get() const
        {
            this->Wait();
            if(this->state_.load(std::memory_order::memory_order_relaxed) == sharpen::FutureState::Completed)
            {
                return this->value_.Get();
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        inline bool CompletedOrError() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state != sharpen::FutureState::Pending;
        }

        inline bool IsPending() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state == sharpen::FutureState::Pending;
        }

        inline bool IsError() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state == sharpen::FutureState::Error;
        }

        inline bool IsCompleted() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state == sharpen::FutureState::Completed;
        }

        inline void SetCallback(Callback &&callback) noexcept
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                if(this->IsPending())
                {
                    this->callback_ = std::move(callback);
                    return;
                }
            }
            if(callback)
            {
                callback(*this);
            }
        }

        inline sharpen::SpinLock &GetCompleteLock() const noexcept
        {
            return *this->lock_;
        }

        inline void Reset() noexcept
        {
            std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
            this->ResetWithoutLock();
        }

    protected:

        virtual void ResetWithoutLock() noexcept
        {
            this->value_.Reset();
            this->error_ = std::exception_ptr{};
            this->state_.store(sharpen::FutureState::Pending,std::memory_order::memory_order_release);
        }

        inline Callback &GetCallback() noexcept
        {
            return this->callback_;
        }

        inline void SetState(sharpen::FutureState state) noexcept
        {
            this->state_.store(state,std::memory_order::memory_order_release);
        }

        virtual void ExecuteCallback(sharpen::FutureState state)
        {
            Callback cb;
            {
                std::unique_lock<sharpen::SpinLock> lock{this->GetCompleteLock()};
                std::swap(cb,this->GetCallback());
                this->SetState(state);
            }
            if(cb)
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
    protected:
        using Callback = std::function<void(Self &)>;
    private:
        std::unique_ptr<sharpen::SpinLock> lock_;
        Callback callback_;
        std::atomic<FutureState> state_;
        std::exception_ptr error_;

    public:

        Future()
            :lock_(new (std::nothrow) sharpen::SpinLock())
            ,callback_()
            ,state_(sharpen::FutureState::Pending)
            ,error_()
        {
            if(!this->lock_)
            {
                throw std::bad_alloc();
            }
        }

        Future(Self &&other) noexcept
            :lock_(std::move(other.lock_))
            ,callback_(std::move(other.callback_))
            ,state_(other.state_.load(std::memory_order::memory_order_relaxed))
            ,error_(std::move(other.error_))
        {
            other.state_.store(sharpen::FutureState::Pending,std::memory_order::memory_order_relaxed);
        }

        virtual ~Future() = default;


        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->callback_ = std::move(other.callback_);
                sharpen::FutureState state{other.state_.load(std::memory_order::memory_order_relaxed)};
                this->state_.store(state,std::memory_order::memory_order_relaxed);
                this->error_ = std::move(other.error_);
                other.state_.store(sharpen::FutureState::Pending,std::memory_order::memory_order_relaxed);
            }
            return *this;
        }

        inline void Complete()
        {
            this->ExecuteCallback(sharpen::FutureState::Completed);
        }

        inline void CompleteForBind()
        {
            this->Complete();
        }

        inline void Fail(std::exception_ptr err)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                this->error_ = std::move(err);
            }
            this->ExecuteCallback(sharpen::FutureState::Error);
        }

        inline void Wait() const
        {
            // std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
            while(this->IsPending())
            {
                // lock.unlock();
                std::this_thread::yield();
                // lock.lock();
            }
        }

        inline void Get() const
        {
            this->Wait();
            if(this->state_.load(std::memory_order::memory_order_relaxed) == sharpen::FutureState::Completed)
            {
                return;
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        inline bool CompletedOrError() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state != sharpen::FutureState::Pending;
        }

        inline bool IsPending() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state == sharpen::FutureState::Pending;
        }

        inline bool IsError() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state == sharpen::FutureState::Error;
        }

        inline bool IsCompleted() const noexcept
        {
            sharpen::FutureState state{this->state_.load(std::memory_order::memory_order_acquire)};
            return state == sharpen::FutureState::Completed;
        }

        void SetCallback(Callback &&callback)
        {
            if(!callback)
            {
                return;
            }
            {
                std::unique_lock<sharpen::SpinLock> lock(*this->lock_);
                if(this->IsPending())
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
            this->error_ = std::exception_ptr{};
            this->state_.store(sharpen::FutureState::Pending,std::memory_order::memory_order_release);
        }

        inline Callback &GetCallback() noexcept
        {
            return this->callback_;
        }

        inline void SetState(sharpen::FutureState state) noexcept
        {
            this->state_.store(state,std::memory_order::memory_order_release);
        }

        inline virtual void ExecuteCallback(sharpen::FutureState state)
        {
            Callback cb;
            {
                std::unique_lock<sharpen::SpinLock> lock{this->GetCompleteLock()};
                std::swap(cb,this->GetCallback());
                this->SetState(state);
            }
            if(cb)
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
        return p;
    }
}

#endif
