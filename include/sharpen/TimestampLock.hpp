#pragma once
#ifndef _SHARPEN_TIMETICK_HPP
#define _SHARPEN_TIMETICK_HPP

#include <unordered_map>
#include <mutex>
#include <memory>

#include "SpinLock.hpp"
#include <cstdint>
#include <cstddef>

namespace sharpen
{
    template<typename _Key>
    class TimestampLock:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::TimestampLock<_Key>;
        using Map = std::unordered_map<_Key,std::uint64_t>;

        std::unique_ptr<sharpen::SpinLock> lock_;
        Map map_;
    public:
    
        TimestampLock()
            :lock_(nullptr)
            ,map_()
        {
            this->lock_.reset(new sharpen::SpinLock{});
            if(!this->lock_)
            {
                throw std::bad_alloc();
            }
        }
    
        TimestampLock(Self &&other) noexcept
            :lock_(std::move(other.lock_))
            ,map_(std::move(other.map_))
        {}
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->map_ = std::move(other.map_);
            }
            return *this;
        }
    
        ~TimestampLock() noexcept = default;

        std::uint64_t GetTimestamp(const _Key &key)
        {
            std::uint64_t time{0};
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                auto pair = this->map_.emplace(key,time);
                if(!pair.second)
                {
                    time = ++(pair.first->first);
                }
            }
            return time;
        }

        bool Valid(const _Key &key,std::uint64_t time)
        {
            std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
            return this->map_[key] == time;
        }
    };
}

#endif