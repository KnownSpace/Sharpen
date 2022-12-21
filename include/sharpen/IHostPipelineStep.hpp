#pragma once
#ifndef _SHARPEN_IHOSTPIPELINESTEP_HPP
#define _SHARPEN_IHOSTPIPELINESTEP_HPP

#include "HostPipelineResult.hpp"
#include "INetStreamChannel.hpp"

namespace sharpen
{
    class IHostPipelineStep
    {
    private:
        using Self = sharpen::IHostPipelineStep;
    protected:
    public:
    
        IHostPipelineStep() noexcept = default;
    
        IHostPipelineStep(const Self &other) noexcept = default;
    
        IHostPipelineStep(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IHostPipelineStep() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::HostPipelineResult Consume(sharpen::INetStreamChannel &channel,const std::atomic_bool &active) noexcept = 0;
    };
}

#endif