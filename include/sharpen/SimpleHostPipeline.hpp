#pragma once
#ifndef _SHARPEN_HOSTPIPELINE_HPP
#define _SHARPEN_HOSTPIPELINE_HPP

#include "IHostPipeline.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <vector>

namespace sharpen
{
    class SimpleHostPipeline
        : public sharpen::IHostPipeline
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::SimpleHostPipeline;

        std::atomic_bool token_;
        std::vector<std::unique_ptr<sharpen::IHostPipelineStep>> pipeline_;

        constexpr static std::size_t reservedPipelineSize_{32};

        virtual void NviConsume(sharpen::NetStreamChannelPtr channel) noexcept override;

        virtual void NviRegister(std::unique_ptr<sharpen::IHostPipelineStep> step) override;

    public:
        SimpleHostPipeline();

        virtual ~SimpleHostPipeline() noexcept;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline virtual bool Active() const noexcept override
        {
            return this->token_;
        }

        inline virtual void Stop() noexcept override
        {
            this->token_ = false;
        }
    };
}   // namespace sharpen

#endif