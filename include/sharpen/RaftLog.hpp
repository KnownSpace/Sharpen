#pragma once
#ifndef _SHARPEN_RAFTLOG_HPP
#define _SHARPEN_RAFTLOG_HPP

#include "ByteBuffer.hpp"
#include "BinarySerializable.hpp"

namespace sharpen
{
    class RaftLog:public sharpen::BinarySerializable<sharpen::RaftLog>
    {
    private:
        using Self = sharpen::RaftLog;
    
        std::uint64_t term_;
        sharpen::ByteBuffer content_;
    public:
    
        RaftLog() noexcept;
    
        RaftLog(std::uint64_t term) noexcept;

        RaftLog(std::uint64_t term,sharpen::ByteBuffer content) noexcept;

        RaftLog(const Self &other) = default;
    
        RaftLog(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~RaftLog() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t GetTerm() const noexcept
        {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept
        {
            this->term_ = term;
        }

        inline sharpen::ByteBuffer &Content() noexcept
        {
            return this->content_;
        }
        
        inline const sharpen::ByteBuffer &Content() const noexcept
        {
            return this->content_;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}

#endif