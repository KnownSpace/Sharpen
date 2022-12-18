#pragma once
#ifndef _SHARPEN_BROADCASTER_HPP
#define _SHARPEN_BROADCASTER_HPP

#include <new>
#include <unordered_map>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "Noncopyable.hpp"
#include "IMailProvider.hpp"
#include "IRemoteActor.hpp"
#include "TypeTraits.hpp"
#include "IteratorOps.hpp"
#include "AsyncMutex.hpp"

namespace sharpen
{
    class Broadcaster:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::Broadcaster;
        using Lock = sharpen::AsyncMutex;
    
        std::unique_ptr<Lock> lock_;
        std::unique_ptr<sharpen::Mail> sharedMail_;
        std::unordered_map<std::uint64_t,std::unique_ptr<sharpen::IRemoteActor>> actors_;
    public:
    
        template<typename _Iterator,typename _Check = decltype(std::declval<std::unique_ptr<sharpen::IRemoteActor>>() = std::move(*std::declval<_Iterator>()++))>
        Broadcaster(std::move_iterator<_Iterator> begin,std::move_iterator<_Iterator> end)
            :lock_(nullptr)
            ,sharedMail_(nullptr)
            ,actors_()
        {
            this->lock_.reset(new (std::nothrow) Lock{});
            this->sharedMail_.reset(new (std::nothrow) sharpen::Mail{});
            if(!this->lock_ || !this->sharedMail_)
            {
                throw std::bad_alloc{};
            }
            std::size_t sz{sharpen::GetRangeSize(begin,end)};
            this->actors_.rehash(sz);
            while (begin != end)
            {
                this->actors_.emplace(begin->GetId(),std::move(*begin));
                ++begin;
            }
        }
    
        Broadcaster(Self &&other) noexcept = default;
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->sharedMail_ = std::move(other.sharedMail_);
                this->actors_ = std::move(other.actors_);
            }
            return *this;
        }
    
        ~Broadcaster() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Broadcast(sharpen::Mail mail);

        void Broadcast(const sharpen::IMailProvider &provider);

        bool Completed() const noexcept;

        void Cancel() noexcept;

        const sharpen::IRemoteActor &GetActor(std::uint64_t id) const;

        bool ExistActor(std::uint64_t id) const noexcept;

        const sharpen::IRemoteActor *FindActor(std::uint64_t id) const noexcept;
    };
}

#endif