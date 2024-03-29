#pragma once
#ifndef _SHARPEN_IREMOTEACTOR_HPP
#define _SHARPEN_IREMOTEACTOR_HPP

#include "ActorId.hpp"
#include "Future.hpp"   // IWYU pragma: keep
#include "IMailParser.hpp"
#include "Mail.hpp"
#include <memory>

namespace sharpen {
    class IRemotePoster {
    private:
        using Self = sharpen::IRemotePoster;

    protected:
        virtual sharpen::ActorId NviGetId() const noexcept = 0;

        // if there are errors occurred
        // return a empty mail
        virtual sharpen::Mail NviPost(const sharpen::Mail &mail) noexcept = 0;

        virtual void NviPost(const sharpen::Mail &mail,
                             std::function<void(sharpen::Mail)> cb) noexcept = 0;

        virtual void NviClose() noexcept = 0;

        virtual void NviOpen(std::unique_ptr<sharpen::IMailParser> parser) = 0;

    public:
        IRemotePoster() noexcept = default;

        IRemotePoster(const Self &other) noexcept = default;

        IRemotePoster(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IRemotePoster() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline void Open(std::unique_ptr<sharpen::IMailParser> parser) {
            assert(parser != nullptr);
            return this->NviOpen(std::move(parser));
        }

        inline void Close() noexcept {
            return this->NviClose();
        }

        // if there are errors occurred
        // return a empty mail
        inline sharpen::Mail Post(const sharpen::Mail &mail) noexcept {
            assert(!mail.Empty());
            return this->NviPost(mail);
        }

        void Post(const sharpen::Mail &mail, std::function<void(sharpen::Mail)> cb) noexcept {
            assert(!mail.Empty());
            assert(cb);
            return this->NviPost(mail, std::move(cb));
        }

        inline sharpen::ActorId GetId() const noexcept {
            return this->NviGetId();
        }

        virtual bool Available() const noexcept = 0;

        virtual bool SupportPipeline() const noexcept = 0;
    };
}   // namespace sharpen

#endif