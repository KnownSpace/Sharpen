#pragma once
#ifndef _SHARPEN_IREMOTEACTORPROPOSER_HPP
#define _SHARPEN_IREMOTEACTORPROPOSER_HPP

#include "Mail.hpp"
#include "RemotePosterStatus.hpp"

namespace sharpen
{
    class IRemotePoster
    {
    private:
        using Self = sharpen::IRemotePoster;
    protected:

    public:
    
        IRemotePoster() noexcept = default;
    
        IRemotePoster(const Self &other) noexcept = default;
    
        IRemotePoster(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRemotePoster() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Post(const sharpen::Mail &mail) = 0;

        virtual void Canel() = 0;

        virtual sharpen::RemotePosterStatus GetStatus() const noexcept = 0;
    };
}

#endif