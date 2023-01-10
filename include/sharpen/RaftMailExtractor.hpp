#pragma once
#ifndef _SHARPEN_RAFTMAILEXTRACTOR_HPP
#define _SHARPEN_RAFTMAILEXTRACTOR_HPP

#include "IRaftMailExtractor.hpp"

namespace sharpen
{
    class RaftMailExtractor:public sharpen::IRaftMailExtractor
    {
    private:
        using Self = sharpen::RaftMailExtractor;
    
        std::uint32_t magic_;

        virtual bool NviIsRaftMail(const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::RaftMailType NviGetMailType(const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftVoteForRequest> NviExtractVoteRequest(const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftVoteForResponse> NviExtractVoteResponse(const sharpen::Mail &mail) const noexcept override;
    public:
    
        explicit RaftMailExtractor(std::uint32_t magic) noexcept;
    
        RaftMailExtractor(const Self &other) noexcept = default;
    
        RaftMailExtractor(Self &&other) noexcept;
    
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
    
        virtual ~RaftMailExtractor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif