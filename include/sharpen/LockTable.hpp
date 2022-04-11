#pragma once
#ifndef _SHARPEN_LOCKTABLE_HPP
#define _SHARPEN_LOCKTABLE_HPP

#include <unordered_map>
#include <memory>
#include <mutex>

#include "SpinLock.hpp"

namespace sharpen
{
    template<typename _Key,typename _Lock>
    class LockTable:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::LockTable<_Key,_Lock>;
        using LockPtr = std::unique_ptr<_Lock>;
        using MapType = std::unordered_map<_Key,LockPtr>;
    
        std::unique_ptr<sharpen::SpinLock> lock_;
        MapType map_;
    public:
    
        LockTable()
            :lock_(nullptr)
            ,map_()
        {
            this->lock_.reset(new sharpen::SpinLock());
            if(!this->lock_)
            {
                throw std::bad_alloc();
            }
        }

        LockTable(Self &&other) noexcept
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

        ~LockTable() noexcept = default;

        template<typename _K,typename _Check = decltype(std::declval<MapType&>().find(std::declval<_K>()))>
        _Lock &GetLock(_K &&key)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                auto ite = this->map_.find(std::forward<_K>(key));
                if(ite != this->map_.end())
                {
                    return *ite->second;
                }
                _Lock *p = new _Lock{};
                if(!p)
                {
                    throw std::bad_alloc();
                }
                this->map_.emplace(std::forward<_K>(key),std::unique_ptr<_Lock>{p});
                return *p;
            }
        }

        template<typename _K,typename _Check = decltype(std::declval<MapType&>().find(std::declval<_K>()))>
        void DeleteLock(_K &&key)
        {
            {
                std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
                auto ite = this->map_.find(std::forward<_K>(key));
                if(ite != this->map_.end())
                {
                    this->map_.erase(ite);
                }
            }
        }
    };
}

#endif