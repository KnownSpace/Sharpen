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
        FutureState state_;
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

        Future(Self &&other) noexcept = default;

        virtual ~Future() = default;

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->value_ = std::move(other.value_);
                this->callback_ = std::move(other.callback_);
                this->state_ = std::move(other.state_);
                this->error_ = std::move(other.error_);
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

        void Wait()
        {
            std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
            while(this->IsPending() && !this->value_.Exist() && !this->error_)
            {
                lock.unlock();
                std::this_thread::yield();
                lock.lock();
            }
        }

        inline _Value &Get()
        {
            this->Wait();
            if(this->state_ == sharpen::FutureState::Completed)
            {
                return this->value_.Get();
            }
            //rethrow exception
            std::rethrow_exception(this->error_);
        }

        inline const _Value &Get() const
        {
            this->Wait();
            if(this->state_ == sharpen::FutureState::Completed)
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
            this->state_ = sharpen::FutureState::Pending;
        }

        inline Callback &GetCallback() noexcept
        {
            return this->callback_;
        }

        inline void SetState(sharpen::FutureState state) noexcept
        {
            this->state_ = state;
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
        FutureState state_;
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

        Future(Self &&other) noexcept = default;

        virtual ~Future() = default;


        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->callback_ = std::move(other.callback_);
                this->state_ = std::move(other.state_);
                this->error_ = std::move(other.error_);
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

        void Fail(std::exception_ptr err)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                this->error_ = std::move(err);
            }
            this->ExecuteCallback(sharpen::FutureState::Error);
        }

        inline void Wait()
        {
            std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
            while(this->IsPending())
            {
                lock.unlock();
                std::this_thread::yield();
                lock.lock();
            }
        }

        inline void Get()
        {
            this->Wait();
            if(this->state_ == sharpen::FutureState::Completed)
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
            this->state_ = sharpen::FutureState::Pending;
        }

        inline Callback &GetCallback() noexcept
        {
            return this->callback_;
        }

        inline void SetState(sharpen::FutureState state) noexcept
        {
            this->state_ = state;
        }

        virtual void ExecuteCallback(sharpen::FutureState state)
        {
            // bool notify{false};
            Callback cb;
            {
                std::unique_lock<sharpen::SpinLock> lock{this->GetCompleteLock()};
                // if(this->GetWaiters() != 0)
                // {
                //     this->SetWaiters(0);
                //     notify = true;
                // }
                std::swap(cb,this->GetCallback());
                this->SetState(state);
            }
            // if(notify)
            // {
            //     this->NotifyAll();
            // }
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
