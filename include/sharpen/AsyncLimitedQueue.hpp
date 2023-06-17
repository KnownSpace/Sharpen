#pragma once
#ifndef _SHARPEN_ASYNCLIMITEDQUEUE_HPP
#define _SHARPEN_ASYNCLIMITEDQUEUE_HPP

#include "AsyncSemaphore.hpp"
#include "Optional.hpp"
#include <cassert>
#include <utility>
#include <vector>

namespace sharpen {
    template<typename _T>
    class AsyncLimitedQueue
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::AsyncLimitedQueue<_T>;

        std::vector<_T> items_;
        sharpen::AsyncSemaphore inbond_;
        sharpen::AsyncSemaphore outbond_;
        std::atomic_uint32_t popIndex_;
        std::atomic_uint32_t pushIndex_;

    public:
        explicit AsyncLimitedQueue(std::uint32_t cap)
            : items_(cap)
            , inbond_(cap)
            , outbond_(0)
            , popIndex_(0)
            , pushIndex_(0) {
            assert(cap != 0);
        }

        ~AsyncLimitedQueue() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        template<typename... _Args, typename _Check = decltype(_T{std::declval<_Args>()...})>
        void Emplace(_Args &&...args) {
            _T item{std::forward<_Args>(args)...};
            this->Push(std::move(item));
        }

        void Push(_T item) noexcept {
            this->inbond_.LockAsync();
            std::uint32_t index{this->pushIndex_.fetch_add(1) % this->items_.size()};
            this->items_[index] = std::move(item);
            this->outbond_.Unlock();
        }

        bool TryPush(_T &&item) noexcept {
            if (!this->inbond_.TryLock()) {
                return false;
            }
            std::uint32_t index{this->pushIndex_.fetch_add(1) % this->items_.size()};
            this->items_[index] = std::move(item);
            this->outbond_.Unlock();
            return true;
        }

        bool TryPush(const _T &item) noexcept {
           _T copy{item};
           return this->TryPush(std::move(item));
        }

        _T Pop() noexcept {
            this->outbond_.LockAsync();
            std::uint32_t index{this->popIndex_.fetch_add(1) % this->items_.size()};
            _T item{std::move(this->items_[index])};
            this->inbond_.Unlock();
            return item;
        }

        sharpen::Optional<_T> TryPop() noexcept {
            if (!this->outbond_.TryLock()) {
                return sharpen::EmptyOpt;
            }
            std::uint32_t index{this->popIndex_.fetch_add(1) % this->items_.size()};
            _T item{std::move(this->items_[index])};
            this->inbond_.Unlock();
            return item;
        }
    };
}   // namespace sharpen

#endif