#pragma once
#ifndef _SHARPEN_ATOMICPTR_HPP
#define _SHARPEN_ATOMICPTR_HPP

#include <thread>
#include <atomic>
#include <vector>
#include <memory>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    //experimental api
    template <typename _Ptr>
    class AtomicPtr : public sharpen::Noncopyable, public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::AtomicPtr<_Ptr>;
        using Storage = std::vector<_Ptr>;

        unsigned char epoch_;
        Storage ptrs_;
        std::atomic_flag flag_;

        static constexpr sharpen::Size bufferSize_ = 3;

    public:
        AtomicPtr()
            :epoch_(0)
            ,ptrs_(bufferSize_)
            ,flag_()
        {
        }

        AtomicPtr(_Ptr ptr)
            : AtomicPtr()
        {
            this->Store(std::move(ptr));
        }

        ~AtomicPtr() noexcept = default;

        _Ptr Load() noexcept
        {
            _Ptr p = this->ptrs_[this->epoch_];
            return p;
        }

        void Store(_Ptr ptr) noexcept
        {
            //weak api
            //write fault
            //other thread writing now
            //give up
            if (this->flag_.test_and_set())
            {
                return;
            }
            char e = (this->epoch_+ 1) % bufferSize_;
            this->ptrs_[e] = std::move(ptr);
            this->epoch_ = e;
            this->flag_.clear();
        }

        _Ptr Exchange(_Ptr ptr) noexcept
        {
            //strong api
            //wait util other thread complete
            while(this->flag_.test_and_set())
            {
                std::this_thread::yield();
            }
            _Ptr old = this->Load();
            char e = (this->epoch_+ 1) % bufferSize_;
            this->ptrs_[e] = std::move(ptr);
            this->epoch_ = e;
            this->flag_.clear();
            return old;
        }

        bool CompareAndSwapWeak(_Ptr &expected, _Ptr desired) noexcept
        {
            //weak api
            if (this->flag_.test_and_set())
            {
                expected = this->Load();
                return false;
            }
            bool r = false;
            auto &&val = this->Load();
            if (val == expected)
            {
                char e = (this->epoch_ + 1) % bufferSize_;
                this->ptrs_[e] = std::move(desired);
                this->epoch_ = e;
                r = true;
            }
            expected = std::move(val);
            this->flag_.clear();
            return r;
        }

        bool CompareAndSwapStrong(_Ptr &expected, _Ptr desired) noexcept
        {
            //strong api
            while(this->flag_.test_and_set())
            {
                std::this_thread::yield();
            }
            bool r = false;
            auto &&val = this->Load();
            if (val == expected)
            {
                char e = (this->epoch_ + 1) % bufferSize_;
                this->ptrs_[e] = std::move(desired);
                this->epoch_ = e;
                r = true;
            }
            expected = val;
            this->flag_.clear();
            return r;
        }

        inline _Ptr load() noexcept
        {
            return this->Load();
        }

        inline void store(_Ptr ptr) noexcept
        {
            return this->Store(std::move(ptr));
        }

        inline _Ptr exchange(_Ptr ptr) noexcept
        {
            return this->Exchange(ptr);
        }

        inline bool compare_and_swap_weak(_Ptr &expected, _Ptr desired) noexcept
        {
            return this->CompareAndSwapWeak(expected, desired);
        }

        inline bool compare_and_swap_strong(_Ptr &expected, _Ptr desired) noexcept
        {
            return this->CompareAndSwapStrong(expected, desired);
        }

        void operator=(_Ptr ptr) noexcept
        {
            this->Store(std::move(ptr));
        }
    };

    template<typename _T>
    using AtomicSharedPtr = sharpen::AtomicPtr<std::shared_ptr<_T>>;
}

#endif