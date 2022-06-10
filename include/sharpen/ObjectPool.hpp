#pragma once
#ifndef _SHARPEN_OBJECTPOOL_HPP
#define _SHARPEN_OBJECTPOOL_HPP

#include <vector>
#include <functional>

#include "Noncopyable.hpp"
#include <cstdint>
#include <cstddef>

namespace sharpen
{
    template<typename _T>
    class ObjectPool:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::ObjectPool<_T>;
        using Storage = std::vector<_T>;
        using Creator = std::function<_T()>;
        using Deletor = std::function<void(_T&)>;
        
        Storage objects_;
        Creator creator_;
        Deletor deletor_;
        std::size_t maxReserved_;
    public:
        ObjectPool(Creator creator,std::size_t maxReserved)
            :objects_()
            ,creator_(std::move(creator))
            ,deletor_()
            ,maxReserved_(maxReserved)
        {}
        
        ObjectPool(Creator creator,Deletor deletor,std::size_t maxReserved)
            :objects_()
            ,creator_(std::move(creator))
            ,deletor_(std::move(deletor))
            ,maxReserved_(maxReserved)
        {}
        
        ObjectPool(Self &&other) noexcept
            :objects_(std::move(other.objects_))
            ,creator_(std::move(other.creator_))
            ,deletor_(std::move(other.deletor_))
            ,maxReserved_(other.maxReserved_)
        {}
        
        ~ObjectPool() noexcept
        {
            for(auto begin = std::begin(this->objects_),end = std::end(this->objects_);begin != end;++begin)
            {
                this->deletor_(*begin);
            }
        }
        
        Self &operator=(Self &&other) noexcept
        {
            this->objects_ = std::move(other.objects_);
            this->creator_ = std::move(other.creator_);
            this->deletor_ = std::move(other.deletor_);
            this->maxReserved = other.maxReserved_;
            return *this;
        }
        
        void Swap(Self &other) noexcept
        {
            if (&other != this)
            {
                std::swap(this->objects_,other.objects_);
                std::swap(this->creator_,other.creator_);
                std::swap(this->deletor_,other.deletor_);
                std::swap(this->maxReserved_,other.maxReserved_);
            }
        }
        
        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }
        
        _T Get() noexcept
        {
            if(this->objects_.empty())
            {
                return std::move(this->creator_());
            }
            _T obj(std::move(this->objects_.back()));
            this->objects_.pop_back();
            return std::move(obj);
        }
        
        void Reserve(std::size_t n)
        {
            for(std::size_t i = 0;i < n;++i)
            {
                this->objects_.push_back(std::move(this->creator()));
            }
        }
        
        void GiveBack(_T &&obj)
        {
            if(this->objects_.size() > this->maxReserved_)
            {
                this->deletor_(obj);
                return;
            }
            this->objects_.push_back(std::move(obj));
        }
    };
}

#endif
