#pragma once
#ifndef _SHARPEN_RAFTMAILBUILDER_HPP
#define _SHARPEN_RAFTMAILBUILDER_HPP

#include "IRaftMailBuilder.hpp"
#include "RaftForm.hpp"

namespace sharpen
{
    class RaftMailBuilder:public sharpen::IRaftMailBuilder
    {
    private:
        using Self = sharpen::RaftMailBuilder;
    
        std::uint32_t magic_;

        sharpen::RaftForm BuildForm(sharpen::RaftMailType type,const sharpen::ByteBuffer &content) const noexcept;

        sharpen::Mail BuildMail(sharpen::RaftMailType type,sharpen::ByteBuffer content) const noexcept;
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

        virtual sharpen::Mail BuildHeartbeatRequest(const sharpen::RaftHeartbeatRequest &request) const override;

        virtual sharpen::Mail BuildHeartbeatResponse(const sharpen::RaftHeartbeatResponse &response) const override;
    
        virtual sharpen::Mail BuildPrevoteRequest(const sharpen::RaftPrevoteRequest &request) const override;

        virtual sharpen::Mail BuildPrevoteResponse(const sharpen::RaftPrevoteResponse &response) const override;

        virtual sharpen::Mail BuildSnapshotRequest(const sharpen::RaftSnapshotRequest &request) const override;

        virtual sharpen::Mail BuildSnapshotResponse(const sharpen::RaftSnapshotResponse &response) const override;
    };
}

#endif