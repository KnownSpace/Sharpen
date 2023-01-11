#pragma once
#ifndef _SHARPEN_IRAFTMAILEXTRACTOR_HPP
#define _SHARPEN_IRAFTMAILEXTRACTOR_HPP

#include <stdexcept>

#include "Mail.hpp"
#include "RaftVoteForRequest.hpp"
#include "RaftVoteForResponse.hpp"
#include "RaftMailType.hpp"
#include "CorruptedDataError.hpp"
#include "RaftHeartbeatRequest.hpp"
#include "RaftHeartbeatResponse.hpp"

namespace sharpen
{
    class IRaftMailExtractor
    {
    private:
        using Self = sharpen::IRaftMailExtractor;
    protected:

        virtual bool NviIsRaftMail(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::RaftMailType NviGetMailType(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::Optional<sharpen::RaftVoteForRequest> NviExtractVoteRequest(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::Optional<sharpen::RaftVoteForResponse> NviExtractVoteResponse(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::Optional<sharpen::RaftHeartbeatRequest> NviExtractHeartbeatRequest(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::Optional<sharpen::RaftHeartbeatResponse> NviExtractHeartbeatResponse(const sharpen::Mail &mail) const noexcept = 0;
    public:
    
        IRaftMailExtractor() noexcept = default;
    
        IRaftMailExtractor(const Self &other) noexcept = default;
    
        IRaftMailExtractor(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftMailExtractor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::RaftMailType GetMailType(const sharpen::Mail &mail) const noexcept
        {
            if(this->IsRaftMail(mail))
            {
                return sharpen::RaftMailType::Unknown;
            }
            return this->NviGetMailType(mail);
        }

        inline bool IsRaftMail(const sharpen::Mail &mail) const noexcept
        {
            if(mail.Empty())
            {
                return false;
            }
            return this->NviIsRaftMail(mail);
        }
    
        inline sharpen::Optional<sharpen::RaftVoteForRequest> ExtractVoteRequest(const sharpen::Mail &mail) const noexcept
        {
            if(!this->IsRaftMail(mail))
            {
                return sharpen::EmptyOpt;
            }
            return this->NviExtractVoteRequest(mail);
        }

        inline sharpen::Optional<sharpen::RaftVoteForResponse> ExtractVoteResponse(const sharpen::Mail &mail) const noexcept
        {
            if(!this->IsRaftMail(mail))
            {
                return sharpen::EmptyOpt;
            }
            return this->NviExtractVoteResponse(mail);
        }  

        inline sharpen::Optional<sharpen::RaftHeartbeatRequest> ExtractHeartbeatRequest(const sharpen::Mail &mail) const noexcept
        {
            if(!this->IsRaftMail(mail))
            {
                return sharpen::EmptyOpt;
            }
            return this->NviExtractHeartbeatRequest(mail);
        }

        inline sharpen::Optional<sharpen::RaftHeartbeatResponse> ExtractHeartbeatResponse(const sharpen::Mail &mail) const noexcept
        {
            if(!this->IsRaftMail(mail))
            {
                return sharpen::EmptyOpt;
            }
            return this->NviExtractHeartbeatResponse(mail);
        }
    };  
} 

#endif