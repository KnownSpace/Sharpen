#pragma once
#ifndef _SHARPEN_ASYNCBLOCKINGQUEUE_HPP
#define _SHARPEN_ASYNCBLOCKINGQUEUE_HPP

#include "AsyncSemaphore.hpp"
#include <deque>

namespace sharpen {
    template<typename _T>
    class AsyncBlockingQueue
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Storage = std::deque<_T>;

        sharpen::SpinLock lock_;
        sharpen::AsyncSemaphore sign_;
        Storage storage_;

    public:
        AsyncBlockingQueue()
            : lock_()
            , sign_(0)
            , storage_() {
        }

        ~AsyncBlockingQueue() noexcept = default;

        _T Pop() {
            this->sign_.LockAsync();
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                _T obj{std::move(this->storage_.front())};
                this->storage_.pop_front();
                return obj;
            }
        }

        void Push(_T obj) {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                this->storage_.push_back(std::move(obj));
            }
            this->sign_.Unlock();
        }

        bool TryPush(_T &&item) noexcept {
            this->Push(std::move(item));
            return true;
        }

        bool TryPush(const _T &item) noexcept {
            _T copy{item};
            return this->TryPush(std::move(item));
        }

        template<typename... _Args, typename _Check = decltype(_T{std::declval<_Args>()...})>
        void Emplace(_Args &&...args) {
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                this->storage_.emplace_back(std::forward<_Args>(args)...);
            }
            this->sign_.Unlock();
        }

        sharpen::Optional<_T> TryPop() noexcept {
            if (!this->sign_.TryLock()) {
                return sharpen::EmptyOpt;
            }
            {
                std::unique_lock<sharpen::SpinLock> lock(this->lock_);
                _T obj{std::move(this->storage_.front())};
                this->storage_.pop_front();
                return obj;
            }
        }
    };
}   // namespace sharpen

#endif