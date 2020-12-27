#pragma once
#ifndef _SHARPEN_CACHEPOOL_HPP
#define _SHARPEN_CACHEPOOL_HPP

#include <vector>
#include <functional>
#include <mutex>

#include "SpinLock.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    template<typename _T>
    class CachePool:public sharpen::Nonmovable,public sharpen::Noncopyable
    {
    private:
        using CleanFunc = std::function<void(_T&)>;
        using Caches = std::vector<_T>
        
        sharpen::SpinLock lock_;
        Caches caches_;
        CleanFunc clean_;
        sharpen::Size limit_;
    public:
        Caches(CleanFunc clean,sharpen::Size limit)
            :lock_()
            ,caches_()
            ,clean_(std::move(clean))
            ,limit_(limit)
        {}
        
        ~Cache() noexcept
        {
            for(auto begin = this->caches_.begin(),end = this->caches_.end();begin != end; ++begin)
            {
                this->clean_(*begin);
            }
        }
        
        void Push(_T obj)
        {
            std::unique_lock<sharpen::SpinLock> lock(this->lock_);
            if(this->caches_.size() < this->limit_)
            {
                this->caches_.push_back(std::move(obj));
                return;
            }
            lock.unlock();
            this->clean_(obj);
        }
        
        bool Pop(_T &obj)
        {
            std::unique_lock<sharpen::SpinLock> lock(this->lock_);
            if(this->caches_.empty())
            {
                return false;
            }
            obj = std::move(this->caches_.back());
            this->caches_.pop_back();
            return true;
        }
    };
}

#endif
