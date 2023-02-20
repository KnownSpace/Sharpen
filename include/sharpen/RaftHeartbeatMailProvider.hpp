#pragma once
#ifndef _SHARPEN_RAFTHEARTBEATMAILPROVIDER_HPP
#define _SHARPEN_RAFTHEARTBEATMAILPROVIDER_HPP

#include <map>

#include "IMailProvider.hpp"
#include "Optional.hpp"
#include "IRaftMailBuilder.hpp"
#include "ILogStorage.hpp"

namespace sharpen
{
    class RaftHeartbeatMailProvider:public sharpen::IMailProvider
    {
    private:
        using Self = sharpen::RaftHeartbeatMailProvider;
    
        std::uint64_t id_;
        const sharpen::IRaftMailBuilder *builder_;
        const sharpen::ILogStorage *log_;
        std::map<std::uint64_t,std::uint64_t> nextIndexs_;
        std::uint64_t term_;
        std::uint64_t commitIndex_;
    public:
    
        RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log);
    
        RaftHeartbeatMailProvider(const Self &other);
    
        RaftHeartbeatMailProvider(Self &&other) noexcept;
    
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
    
        virtual ~RaftHeartbeatMailProvider() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void PrepareTerm(std::uint64_t term) noexcept;

        void PrepareCommitIndex(std::uint64_t commitIndex) noexcept;

        virtual sharpen::Mail Provide(std::uint64_t actorId) const;

        sharpen::Optional<std::uint64_t> LookupIndex(std::uint64_t actorId) const noexcept;

        void SetIndex(std::uint64_t actorId,std::uint64_t index);

        sharpen::Optional<std::uint64_t> GetSynchronizedIndex() const noexcept;

        sharpen::Mail ProvideSynchronizedMail() const;

        void RemoveIndex(std::uint64_t actorId) noexcept;

        std::size_t GetSize() const noexcept;

        bool Empty() const noexcept;
    };
}

#endif