#pragma once
#ifndef _SHARPEN_RAFTVOTEFORREQUEST_HPP
#define _SHARPEN_RAFTVOTEFORREQUEST_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#include "BinarySerializable.hpp"
#include "CorruptedDataError.hpp"

namespace sharpen
{
    class RaftVoteForRequest:public sharpen::BinarySerializable<sharpen::RaftVoteForRequest>
    {
    private:
        using Self = sharpen::RaftVoteForRequest;
    
        std::uint64_t id_;
        std::uint64_t term_;
        std::uint64_t lastIndex_;
        std::uint64_t lastTerm_;
    public:
    
        RaftVoteForRequest() noexcept = default;

        RaftVoteForRequest(std::uint64_t id,std::uint64_t term,std::uint64_t lastIndex,std::uint64_t lastTerm) noexcept;
    
        RaftVoteForRequest(const Self &other) noexcept = default;
    
        RaftVoteForRequest(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other) noexcept
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~RaftVoteForRequest() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        inline std::uint64_t GetId() const noexcept
        {
            return this->id_;
        }

        inline void SetId(std::uint64_t id) noexcept
        {
            this->id_ = id;
        }

        inline std::uint64_t GetTerm() const noexcept
        {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept
        {
            this->term_ = term;
        }

        inline std::uint64_t GetLastIndex() const noexcept
        {
            return this->lastIndex_;
        }

        inline void SetLastIndex(std::uint64_t lastIndex) noexcept
        {
            this->lastIndex_ = lastIndex;
        }

        inline std::uint64_t GetLastTerm() const noexcept
        {
            return this->lastTerm_;
        }

        inline void SetLastTerm(std::uint64_t lastTerm) noexcept
        {
            this->lastTerm_ = lastTerm;
        }
    };
}

#endif