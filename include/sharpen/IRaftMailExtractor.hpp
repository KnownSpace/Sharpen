#pragma once
#ifndef _SHARPEN_IRAFTMAILEXTRACTOR_HPP
#define _SHARPEN_IRAFTMAILEXTRACTOR_HPP

#include <stdexcept>

#include "Mail.hpp"
#include "RaftVoteForRequest.hpp"
#include "RaftVoteForResponse.hpp"
#include "RaftMailType.hpp"
#include "CorruptedDataError.hpp"

namespace sharpen
{
    class IRaftMailExtractor
    {
    private:
        using Self = sharpen::IRaftMailExtractor;
    protected:

        virtual bool NviIsRaftMail(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::RaftMailType NviGetMailType(const sharpen::Mail &mail) const noexcept = 0;

        virtual sharpen::RaftVoteForRequest NviExtractVoteRequest(const sharpen::Mail &mail) const = 0;

        virtual sharpen::RaftVoteForResponse NviExtractVoteResponse(const sharpen::Mail &mail) const = 0;
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
    
        inline sharpen::RaftVoteForRequest ExtractVoteRequest(const sharpen::Mail &mail) const
        {
            if(!this->IsRaftMail(mail))
            {
                throw std::invalid_argument{"unexpected mail"};
            }
            return this->NviExtractVoteRequest(mail);
        }

        inline sharpen::RaftVoteForResponse ExtractVoteResponse(const sharpen::Mail &mail) const
        {
            if(!this->IsRaftMail(mail))
            {
                throw std::invalid_argument{"unexpected mail"};
            }
            return this->NviExtractVoteResponse(mail);
        }  
    };  
} 

#endif