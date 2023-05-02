#pragma once
#ifndef _SHARPEN_IMAILPROVIDER_HPP
#define _SHARPEN_IMAILPROVIDER_HPP

#include "Mail.hpp"

namespace sharpen {
    class IMailProvider {
    private:
        using Self = sharpen::IMailProvider;

    protected:
    public:
        IMailProvider() noexcept = default;

        IMailProvider(const Self &other) noexcept = default;

        IMailProvider(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IMailProvider() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual sharpen::Mail Provide(std::uint64_t actorId) const = 0;
    };
}   // namespace sharpen

#endif