#pragma once
#ifndef _SHARPEN_ICONSENSUS_HPP
#define _SHARPEN_ICONSENSUS_HPP

#include <memory>

#include "ILogBatch.hpp"
#include "ILogStorage.hpp"
#include "IMailReceiver.hpp"
#include "AwaitableFuture.hpp"
#include "IMailReceiver.hpp"

namespace sharpen
{
    class IConsensus
    {
    private:
        using Self = sharpen::IConsensus;
    protected:

        //return current index
        virtual void NviStatusChanged(sharpen::Future<std::uint64_t> &future,std::uint64_t minIndex) = 0;
    
        virtual bool NviIsConsensusMail(const sharpen::Mail &mail) const noexcept = 0;

        //return the last index of log batch
        virtual std::uint64_t NviWrite(std::unique_ptr<sharpen::ILogBatch> logs) = 0;

        virtual sharpen::Mail NviGenerateResponse(sharpen::Mail request) = 0;

        virtual void NviDropLogsUntil(std::uint64_t index) = 0;
    public:

        IConsensus() noexcept = default;
    
        IConsensus(const Self &other) noexcept = default;
    
        IConsensus(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IConsensus() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Advance() = 0;

        virtual bool Writable() const = 0;

        virtual std::unique_ptr<sharpen::ILogBatch> CreateLogBatch() const = 0;

        inline std::uint64_t Write(std::unique_ptr<sharpen::ILogBatch> logs)
        {
            if(logs != nullptr)
            {
                return this->NviWrite(std::move(logs));
            }
            return 0;
        }

        virtual bool Changable() const = 0;

        inline void StatusChanged(sharpen::Future<std::uint64_t> &future,std::uint64_t minIndex)
        {
            this->NviStatusChanged(future,minIndex);
        }

        inline void StatusChanged(sharpen::Future<std::uint64_t> &future)
        {
            this->NviStatusChanged(future,sharpen::ILogStorage::noneIndex);
        }

        inline std::uint64_t StatusChanged(std::uint64_t minIndex)
        {
            sharpen::AwaitableFuture<std::uint64_t> future;
            this->NviStatusChanged(future,minIndex);
            return future.Await();
        }

        inline std::uint64_t StatusChanged()
        {
             sharpen::AwaitableFuture<std::uint64_t> future;
            this->NviStatusChanged(future,sharpen::ILogStorage::noneIndex);
            return future.Await();
        }

        virtual const sharpen::ILogStorage &ImmutableLogs() const noexcept = 0;

        inline bool IsConsensusMail(const sharpen::Mail &mail) const noexcept
        {
            if(mail.Empty())
            {
                return false;
            }
            return this->NviIsConsensusMail(mail);
        }

        virtual sharpen::IMailReceiver &GetReceiver() noexcept = 0;

        virtual const sharpen::IMailReceiver &GetReceiver() const noexcept = 0;

        inline sharpen::Mail GenerateResponse(sharpen::Mail request)
        {
            if(!request.Empty() && this->IsConsensusMail(request))
            {
                return this->NviGenerateResponse(std::move(request));
            }
            return sharpen::Mail{};
        }

        inline void DropLogsUntil(std::uint64_t index)
        {
            this->NviDropLogsUntil(index);
        }
    };   
}

#endif