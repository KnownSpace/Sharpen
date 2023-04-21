#pragma once
#ifndef _SHARPEN_IHOST_HPP
#define _SHARPEN_IHOST_HPP

#include "IHostPipeline.hpp"
#include "TypeTraits.hpp"
#include <cassert>
#include <memory>

namespace sharpen
{
    class IHost
    {
    private:
        using Self = sharpen::IHost;

    protected:
        virtual void NviSetPipeline(std::unique_ptr<sharpen::IHostPipeline> pipeline) noexcept = 0;

    public:
        IHost() noexcept = default;

        IHost(const Self &other) noexcept = default;

        IHost(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IHost() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Run() = 0;

        virtual void Stop() noexcept = 0;

        inline void SetPipeline(std::unique_ptr<sharpen::IHostPipeline> pipeline) noexcept
        {
            assert(pipeline != nullptr);
            this->NviSetPipeline(std::move(pipeline));
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<std::unique_ptr<sharpen::IHostPipeline>,
                                                          _Fn,
                                                          _Args...>::Value>>
        inline void ConfiguratePipeline(_Fn &&fn, _Args &&...args)
        {
            std::unique_ptr<sharpen::IHostPipeline> pipeline{
                std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...)()};
            this->SetPipeline(std::move(pipeline));
        }
    };
}   // namespace sharpen

#endif