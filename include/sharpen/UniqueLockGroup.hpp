#pragma once
#ifndef _SHARPEN_UNIQUELOCKGROUP_HPP
#define _SHARPEN_UNIQUELOCKGROUP_HPP

#include <mutex>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <new>

#include "Noncopyable.hpp"

namespace sharpen
{
    template<typename _Lock>
    class UniqueLockGroup:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::UniqueLockGroup<_Lock>;
        using UniqueLock = std::unique_lock<_Lock>;

        UniqueLock *locks_;
        std::size_t size_;
        std::size_t beginIndex_;
        std::size_t endIndex_;
    public:

        explicit UniqueLockGroup(std::size_t size)
            :locks_(nullptr)
            ,size_(0)
            ,beginIndex_(0)
            ,endIndex_(0)
        {
            assert(size);
            UniqueLock *locks{reinterpret_cast<UniqueLock *>(std::calloc(size,sizeof(*locks)))};
            if(!locks)
            {
                throw std::bad_alloc{};
            }
            this->locks_ = locks;
            this->size_ = size;
        }

        UniqueLockGroup(Self &&other) noexcept
            :locks_(other.locks_)
            ,size_(other.size_)
            ,beginIndex_(other.beginIndex_)
            ,endIndex_(other.endIndex_)
        {
            other.locks_ = nullptr;
            other.size_ = 0;
            other.beginIndex_ = 0;
            other.endIndex_ = 0;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->Clear();
                if(this->locks_)
                {
                    std::free(this->locks_);
                }
                this->locks_ = other.locks_;
                this->size_ = other.size_;
                this->beginIndex_ = other.beginIndex_;
                this->endIndex_ = other.endIndex_;
                other.locks_ = nullptr;
                other.size_ = 0;
                other.beginIndex_ = 0;
                other.endIndex_ = 0;
            }
            return *this;
        }

        ~UniqueLockGroup() noexcept
        {
            this->Clear();
            if(this->locks_)
            {
                std::free(this->locks_);
            }
        }

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::size_t GetMaxSize() const noexcept
        {
            return this->size_;
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->endIndex_ - this->beginIndex_;
        }

        inline void Clear() noexcept
        {
            this->Clear(this->endIndex_ - this->beginIndex_);
        }

        inline void Clear(std::size_t size) noexcept
        {
            if(this->locks_)
            {
                std::size_t endIndex = this->beginIndex_ + size;
                assert(this->endIndex_ >= endIndex);
                for(std::size_t i = this->beginIndex_;i != endIndex;++i)
                {
                    td::size_t index{i % this->size_};
                    this->locks_[index].~unique_lock();
                }
                this->beginIndex_ = endIndex;
            }
        }

        inline void Lock(UniqueLock &&lock) noexcept
        {
            assert(this->endIndex_ - this->beginIndex_ != this->size_);
            std::size_t index{this->endIndex_};
            this->endIndex_ += 1;
            new (this->locks_ + index) UniqueLock{std::move(lock)};
        }
    };
}

#endif