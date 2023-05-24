#pragma once
#ifndef _MAILTASK_HPP
#define _MAILTASK_HPP

#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/Mail.hpp>

class MailTask : public sharpen::Noncopyable {
private:
    using Self = MailTask;

    sharpen::INetStreamChannel *channel_;
    sharpen::Mail mail_;
    sharpen::ChannelPtr pin_;

public:
    MailTask(sharpen::INetStreamChannel &channel, sharpen::Mail mail) noexcept
        : channel_(&channel)
        , mail_(std::move(mail))
        , pin_(channel.shared_from_this()) {
    }

    MailTask(Self &&other) noexcept
        : channel_(other.channel_)
        , mail_(std::move(other.mail_))
        , pin_(std::move(other.pin_)) {
        other.channel_ = nullptr;
    }

    inline Self &operator=(Self &&other) noexcept {
        if (this != std::addressof(other)) {
            this->channel_ = other.channel_;
            this->mail_ = std::move(other.mail_);
            this->pin_ = std::move(other.pin_);
            other.channel_ = nullptr;
        }
        return *this;
    }

    ~MailTask() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline sharpen::INetStreamChannel &Channel() noexcept {
        return *this->channel_;
    }

    inline const sharpen::INetStreamChannel &Channel() const noexcept {
        return *this->channel_;
    }

    inline sharpen::Mail &Mail() noexcept {
        return this->mail_;
    }

    inline const sharpen::Mail &Mail() const noexcept {
        return this->mail_;
    }
};

#endif