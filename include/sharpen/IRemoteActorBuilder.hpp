#pragma once
#ifndef _SHARPEN_IREMOTEACTORBUILDER_HPP
#define _SHARPEN_IREMOTEACTORBUILDER_HPP

#include <memory>

#include "IRemoteActor.hpp"

namespace sharpen
{
    class IRemoteActorBuilder
    {
    private:
        using Self = sharpen::IRemoteActorBuilder;
    protected:

        virtual std::unique_ptr<sharpen::IRemoteActor> NviBuild() const = 0;

        virtual std::shared_ptr<sharpen::IRemoteActor> NviBuildShared() const = 0;
    public:
    
        IRemoteActorBuilder() noexcept = default;
    
        IRemoteActorBuilder(const Self &other) noexcept = default;
    
        IRemoteActorBuilder(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRemoteActorBuilder() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        std::unique_ptr<sharpen::IRemoteActor> Build() const
        {
            return this->Build();
        }

        std::shared_ptr<sharpen::IRemoteActor> BuildShared() const
        {
            return this->BuildShared();
        }
    };
}

#endif