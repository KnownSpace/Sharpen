#pragma once
#ifndef _SHARPEN_IRAFTMAILBUILDER_HPP
#define _SHARPEN_IRAFTMAILBUILDER_HPP

#include "Mail.hpp"
#include "RaftVoteForRequest.hpp"

namespace sharpen
{
    class IRaftMailBuilder
    {
    private:
        using Self = sharpen::IRaftMailBuilder;
    protected:
    public:
    
        IRaftMailBuilder() noexcept = default;
    
        IRaftMailBuilder(const Self &other) noexcept = default;
    
        IRaftMailBuilder(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftMailBuilder() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::Mail BuildVoteRequest(const sharpen::RaftVoteForRequest &request) const = 0;
    
        virtual sharpen::Mail BuildVoteResponse(const sharpen::RaftVoteForRequest &response) const = 0;
    };
}

#endif