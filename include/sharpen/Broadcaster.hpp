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
        std::size_t index_;
        std::size_t pipelineLength_;
        std::vector<sharpen::Mail> sharedMails_;
        std::unordered_map<std::uint64_t,std::unique_ptr<sharpen::IRemoteActor>> actors_;

        sharpen::Mail *GetNextSharedMail() noexcept;
    public:
    
        template<typename _Iterator,typename _Check = decltype(std::declval<std::unique_ptr<sharpen::IRemoteActor>&>() = std::move(*std::declval<_Iterator&>()++))>
        Broadcaster(std::move_iterator<_Iterator> begin,std::move_iterator<_Iterator> end)
            :Self{begin,end,1}
        {}

        template<typename _Iterator,typename _Check = decltype(std::declval<std::unique_ptr<sharpen::IRemoteActor>&>() = std::move(*std::declval<_Iterator&>()++))>
        Broadcaster(std::move_iterator<_Iterator> begin,std::move_iterator<_Iterator> end,std::size_t pipeline)
            :lock_(nullptr)
            ,index_(0)
            ,pipelineLength_(pipeline)
            ,sharedMails_()
            ,actors_()
        {
            this->lock_.reset(new (std::nothrow) Lock{});
            if(!this->lock_)
            {
                throw std::bad_alloc{};
            }
            std::size_t sz{sharpen::GetRangeSize(begin,end)};
            this->actors_.rehash(sz);
            while (begin != end)
            {
                std::uint64_t id{(*begin)->GetId()};
                this->actors_.emplace(id,std::move(*begin));
                ++begin;
            }
            assert(this->pipelineLength_ > 0);
            this->sharedMails_.resize(this->pipelineLength_);
        }

        Broadcaster(std::unique_ptr<sharpen::IRemoteActor> *actors,std::size_t size,std::size_t pipeline);

        Broadcaster(std::unique_ptr<sharpen::IRemoteActor> *actors,std::size_t size)
            :Self{actors,size,1}
        {}
    
        Broadcaster(Self &&other) noexcept = default;
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->lock_ = std::move(other.lock_);
                this->index_ = other.index_;
                this->pipelineLength_ = other.pipelineLength_;
                this->sharedMails_ = std::move(other.sharedMails_);
                this->actors_ = std::move(other.actors_);
                other.index_ = 0;
                other.pipelineLength_ = 0;
            }
            return *this;
        }
    
        ~Broadcaster() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Broadcast(sharpen::Mail mail);

        void Broadcast(const sharpen::IMailProvider &provider);

        bool Completed() const noexcept;

        void Cancel() noexcept;

        void Drain() noexcept;

        const sharpen::IRemoteActor &GetActor(std::uint64_t id) const;

        bool ExistActor(std::uint64_t id) const noexcept;

        const sharpen::IRemoteActor *FindActor(std::uint64_t id) const noexcept;

        std::size_t GetPipelineLength() const noexcept;

        bool SupportPipeline() const noexcept;
    };
}

#endif