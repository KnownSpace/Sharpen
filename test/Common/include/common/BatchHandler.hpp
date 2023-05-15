#pragma once
#ifndef _BATCHHANDLER_HPP
#define _BATCHHANDLER_HPP

#include "MailTask.hpp"
#include <sharpen/AsyncMutex.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/Mail.hpp>
#include <vector>

class BatchHandler
    : public sharpen::Noncopyable
    , public sharpen::Nonmovable {
private:
    using Self = BatchHandler;

    sharpen::AsyncMutex lock_;
    std::vector<MailTask> tasks_;
    std::uint32_t batch_;
    std::function<void(MailTask*)> handler_;

public:
    BatchHandler(std::uint32_t batch,std::function<void(MailTask*)> handler) noexcept;

    ~BatchHandler() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    void operator()(sharpen::INetStreamChannel *channel, sharpen::Mail mail) noexcept;
};

class BatchHandlerWrap
{
private:
    using Self = BatchHandlerWrap;

    std::shared_ptr<BatchHandler> impl_;
public:

    BatchHandlerWrap(std::uint32_t batch,std::function<void(MailTask*)> handler)
        :impl_(std::make_shared<BatchHandler>(batch,std::move(handler)))
    {}

    BatchHandlerWrap(const Self &other) = default;

    BatchHandlerWrap(Self &&other) noexcept = default;

    inline Self &operator=(const Self &other)
    {
        if(this != std::addressof(other))
        {
            Self tmp{other};
            std::swap(tmp,*this);
        }
        return *this;
    }

    Self &operator=(Self &&other) noexcept = default;

    ~BatchHandlerWrap() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }
    inline void operator()(sharpen::INetStreamChannel *channel, sharpen::Mail mail) noexcept {
        return (*this->impl_)(channel,std::move(mail));
    }
};

#endif