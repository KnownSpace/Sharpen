#pragma once
#ifndef _SHARPEN_FUNCTORHOSTPIPELINE_HPP
#define _SHARPEN_FUNCTORHOSTPIPELINE_HPP

#include <functional>
#include <vector>

#include "IHostPipeline.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class FunctorHostPipeline
        : public sharpen::IHostPipeline
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = FunctorHostPipeline;
        using Base = sharpen::IHostPipeline;
        using Functor = std::function<sharpen::HostPipelineResult(sharpen::INetStreamChannel &,
                                                                  const std::atomic_bool &)>;

        std::atomic_bool token_;
        std::vector<std::unique_ptr<sharpen::IHostPipelineStep>> pipeline_;
        std::vector<Functor> functors_;

        constexpr static std::size_t reservedPipelineSize_{32};

        virtual void NviConsume(sharpen::NetStreamChannelPtr channel) noexcept override;

        virtual void NviRegister(std::unique_ptr<sharpen::IHostPipelineStep> step) override;

    public:
        FunctorHostPipeline();

        virtual ~FunctorHostPipeline() noexcept;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        Self &Register(Functor func);

        template<typename _Impl,
                 typename... _Args,
                 typename _Check = decltype(
                     std::declval<sharpen::IHostPipelineStep *&>() = std::declval<_Impl *>(),
                     _Impl{std::declval<_Args>()...})>
        inline Self &Register(_Args &&...args)
        {
            Base::Register<_Impl>(std::forward<_Args>(args)...);
            return *this;
        }

        inline Self &Register(std::unique_ptr<sharpen::IHostPipelineStep> step)
        {
            Base::Register(std::move(step));
            return *this;
        }
    };
}   // namespace sharpen

#endif