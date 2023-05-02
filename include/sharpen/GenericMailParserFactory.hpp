#pragma once
#ifndef _SHARPEN_GENERICMAILPARSERFACTORY_HPP
#define _SHARPEN_GENERICMAILPARSERFACTORY_HPP

#include "IMailParser.hpp"
#include "IMailParserFactory.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class GenericMailParserFactory : public sharpen::IMailParserFactory {
    private:
        using Self = sharpen::GenericMailParserFactory;

        std::uint32_t magic_;

    public:
        explicit GenericMailParserFactory(std::uint32_t magic) noexcept;

        GenericMailParserFactory(const Self &other) noexcept = default;

        GenericMailParserFactory(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual ~GenericMailParserFactory() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual std::unique_ptr<sharpen::IMailParser> Produce() override;
    };
}   // namespace sharpen

#endif