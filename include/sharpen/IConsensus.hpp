#pragma once
#ifndef _SHARPEN_ICONSENSUS_HPP
#define _SHARPEN_ICONSENSUS_HPP

#include "IRpcProvider.hpp"
#include "IRpcInvokerFactory.hpp"

namespace sharpen
{
    class IConsensus
    {
    private:
        using Self = sharpen::IConsensus;
    protected:
    public:
    
        enum class AdvanceResult
        {
            Success,
            Fail
        };

        IConsensus() noexcept = default;
    
        IConsensus(const Self &other) noexcept = default;
    
        IConsensus(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IConsensus() noexcept = default;

        virtual void RegisterRpcs(sharpen::IRpcProvider &provider) = 0;

        virtual void SetRpcInvokerFactory(std::unique_ptr<sharpen::IRpcInvokerFactory> factory) = 0;

        virtual void Propose(const sharpen::ByteBuffer &proposal) = 0;

        virtual AdvanceResult Advance() = 0;

        virtual void Wait() = 0;

        virtual void GetCommitedProposes() = 0;

        virtual void AppilTo(std::uint64_t index) = 0;
    };
}

#endif