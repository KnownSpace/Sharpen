#pragma once
#ifndef _SHARPEN_RAFTMAILBUILDER_HPP
#define _SHARPEN_RAFTMAILBUILDER_HPP

#include "IRaftMailBuilder.hpp"

namespace sharpen
{
    class RaftMailBuilder:public sharpen::IRaftMailBuilder
    {
    private:
        using Self = sharpen::RaftMailBuilder;
    
        std::uint32_t magic_;
    public:
    
        explicit RaftMailBuilder(std::uint32_t magic) noexcept;
    
        RaftMailBuilder(const Self &other) noexcept = default;
    
        RaftMailBuilder(Self &&other) noexcept;
    
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
    
        virtual ~RaftMailBuilder() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::Mail BuildVoteRequest(const sharpen::RaftVoteForRequest &request) const override;
    
        virtual sharpen::Mail BuildVoteResponse(const sharpen::RaftVoteForResponse &response) const override;
    };
}

#endif