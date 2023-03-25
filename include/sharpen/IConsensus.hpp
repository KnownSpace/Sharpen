#pragma once
#ifndef _SHARPEN_ICONSENSUS_HPP
#define _SHARPEN_ICONSENSUS_HPP

#include <memory>

#include "ILogBatch.hpp"
#include "ILogStorage.hpp"
#include "IMailReceiver.hpp"
#include "AwaitableFuture.hpp"
#include "IMailReceiver.hpp"
#include "IQuorum.hpp"

namespace sharpen
{
    class IConsensus
    {
    private:
        using Self = sharpen::IConsensus;
    protected:

        //returns current advanced count
        virtual void NviWaitNextConsensus(sharpen::Future<std::uint64_t> &future,std::uint64_t advancedCount) = 0;
    
        virtual bool NviIsConsensusMail(const sharpen::Mail &mail) const noexcept = 0;

        //returns the last index of log batch
        virtual std::uint64_t NviWrite(std::unique_ptr<sharpen::ILogBatch> logs) = 0;

        virtual sharpen::Mail NviGenerateResponse(sharpen::Mail request) = 0;

        virtual void NviDropLogsUntil(std::uint64_t index) = 0;

        virtual void NviConfigurateQuorum(std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum*)> configurater) = 0;
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

        //use it like:
        //std::uint64_t count{0};
        //while(1)
        //{
        //  count = consensus.WaitNextConsensus(count);  
        //}

        //returns current advanced count
        inline void WaitNextConsensus(std::uint64_t advancedCount,sharpen::Future<std::uint64_t> &future)
        {
            this->NviWaitNextConsensus(future,advancedCount);
        }

        //returns current advanced count
        inline std::uint64_t WaitNextConsensus(std::uint64_t advancedCount)
        {
            sharpen::AwaitableFuture<std::uint64_t> future;
            this->NviWaitNextConsensus(future,advancedCount);
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

        inline void ConfigurateQuorum(std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum*)> configurater)
        {
            if(configurater)
            {
                this->NviConfigurateQuorum(std::move(configurater));
            }
        }

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<std::unique_ptr<sharpen::IQuorum>,_Fn,sharpen::IQuorum*,_Args...>::Value>>
        inline void ConfigurateQuorum(_Fn &&fn,_Args &&...args)
        {
            std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum*)> config{std::bind(std::forward<_Fn>(fn),std::placeholders::_1,std::forward<_Args>(args)...)};
            this->ConfigurateQuorum(std::move(config));
        }

        virtual sharpen::Optional<std::uint64_t> GetWriterId() const noexcept = 0;
    };   
}

#endif