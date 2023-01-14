#pragma once
#ifndef _SHARPEN_RAFTLEADERRECORD_HPP
#define _SHARPEN_RAFTLEADERRECORD_HPP

#include <cstdint>
#include <cstddef>
#include <utility>
#include <atomic>

#include "Optional.hpp"

namespace sharpen
{
    class RaftLeaderRecord
    {
    private:
        using Self = sharpen::RaftLeaderRecord;
    
        std::atomic_uint64_t term_;
        std::atomic_uint64_t leaderId_;
    public:
    
        RaftLeaderRecord() noexcept;

        RaftLeaderRecord(std::uint64_t term,std::uint64_t leaderId) noexcept;
    
        RaftLeaderRecord(const Self &other) noexcept;
    
        RaftLeaderRecord(Self &&other) noexcept;
    
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
    
        ~RaftLeaderRecord() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t GetTerm() const noexcept
        {
            return this->term_;
        }

        bool ExistLeader() const noexcept
        {
            return this->term_ != 0;
        }

        inline sharpen::Optional<std::uint64_t> GetLeaderId() const noexcept
        {
            if(!this->ExistLeader())
            {
                return sharpen::EmptyOpt;
            }
            return this->leaderId_;
        }

        void Flush(std::uint64_t term,std::uint64_t leaderId) noexcept;
    };
}

#endif