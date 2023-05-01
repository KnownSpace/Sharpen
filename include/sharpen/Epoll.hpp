#pragma once
#ifndef _SHARPEN_EPOLL_HPP
#define _SHARPEN_EPOLL_HPP

#include "SystemMacro.hpp"   // IWYU pragma: keep

// epoll is only support by linux
#ifdef SHARPEN_IS_LINUX

#define SHARPEN_HAS_EPOLL

#include "FileTypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>

namespace sharpen {
    class Epoll
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        sharpen::FileHandle handle_;

    public:
        using Event = epoll_event;

        Epoll();

        ~Epoll() noexcept;

        std::uint32_t Wait(Event *events, std::uint32_t maxEvents, std::int32_t timeout);

        void Add(sharpen::FileHandle handle, Event *event);

        void Remove(sharpen::FileHandle handle);

        void Update(sharpen::FileHandle handle, Event *event);
    };
}   // namespace sharpen

#endif
#endif
