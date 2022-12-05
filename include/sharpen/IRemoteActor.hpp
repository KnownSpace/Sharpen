#pragma once
#ifndef _SHARPEN_IREMOTEACTORPROPOSER_HPP
#define _SHARPEN_IREMOTEACTORPROPOSER_HPP

#include "Mail.hpp"
#include "RemoteActorStatus.hpp"

namespace sharpen
{
    class IRemoteActor
    {
    private:
        using Self = sharpen::IRemoteActor;
    protected:

        virtual std::uint64_t DoGetId() const noexcept = 0;
    public:
    
        IRemoteActor() noexcept = default;
    
        IRemoteActor(const Self &other) noexcept = default;
    
        IRemoteActor(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRemoteActor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Post(sharpen::Mail mail) = 0;

        virtual void PostShared(const sharpen::Mail &mail) = 0;

        virtual sharpen::RemoteActorStatus GetStatus() const noexcept = 0;

        virtual void Cancel() noexcept = 0;

        inline std::uint64_t GetId() const noexcept
        {
            return this->DoGetId();
        }
    };
}

#endif