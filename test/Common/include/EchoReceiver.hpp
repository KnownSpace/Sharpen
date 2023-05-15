#pragma once
#ifndef _ECHORECEIVER_HPP
#define _ECHORECEIVER_HPP

#include <sharpen/IMailReceiver.hpp>
#include <sharpen/Noncopyable.hpp>
#include <sharpen/Nonmovable.hpp>
#include <atomic>


class EchoReceiver
    : public sharpen::IMailReceiver
    , public sharpen::Noncopyable
    , public sharpen::Nonmovable {
private:
    using Self = EchoReceiver;

    std::atomic_size_t counter_;

    virtual void NviReceive(sharpen::Mail mail, std::uint64_t actorId) override;

public:
    EchoReceiver() noexcept;

    virtual ~EchoReceiver() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline std::size_t GetCount() const noexcept {
        return this->counter_.load(std::memory_order_relaxed);
    }
};

#endif