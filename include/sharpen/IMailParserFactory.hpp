#pragma once
#ifndef _SHARPEN_IMAILPARSERFACTORY_HPP
#define _SHARPEN_IMAILPARSERFACTORY_HPP

#include "IMailParser.hpp"

namespace sharpen {
    class IMailParserFactory {
    private:
        using Self = sharpen::IMailParserFactory;

    protected:
    public:
        IMailParserFactory() noexcept = default;

        IMailParserFactory(const Self &other) noexcept = default;

        IMailParserFactory(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IMailParserFactory() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual std::unique_ptr<sharpen::IMailParser> Produce() = 0;
    };
}   // namespace sharpen

#endif