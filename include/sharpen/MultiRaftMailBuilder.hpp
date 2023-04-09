#pragma once
#ifndef _SHARPEN_MULTIRAFTMAILBUILDER_HPP
#define _SHARPEN_MULTIRAFTMAILBUILDER_HPP

#include <cstdint>
#include <cstddef>

#include "IRaftMailBuilder.hpp"
#include "MultiRaftForm.hpp"

namespace sharpen
{
    class MultiRaftMailBuilder:public sharpen::IRaftMailBuilder
    {
    private:
        using Self = sharpen::MultiRaftMailBuilder;
    
        std::uint32_t magic_;
        std::uint32_t raftNumber_;

        sharpen::MultiRaftForm BuildForm(sharpen::RaftMailType type,const sharpen::ByteBuffer &content) const noexcept;
    
        sharpen::Mail BuildMail(sharpen::RaftMailType type,sharpen::ByteBuffer content) const noexcept;
    public:
    
        MultiRaftMailBuilder(std::uint32_t magic,std::uint32_t raftNumber) noexcept;
    
        MultiRaftMailBuilder(const Self &other) noexcept = default;
    
        MultiRaftMailBuilder(Self &&other) noexcept;
    
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
    
        virtual ~MultiRaftMailBuilder() noexcept = default;
    
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