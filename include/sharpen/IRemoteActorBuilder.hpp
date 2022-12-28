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

        virtual std::unique_ptr<sharpen::IRemoteActor> Build() const = 0;

        virtual std::shared_ptr<sharpen::IRemoteActor> BuildShared() const = 0;
    };
}

#endif