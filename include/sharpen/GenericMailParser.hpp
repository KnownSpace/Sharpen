#pragma once
#ifndef _SHARPEN_GENERICMAILPASER_HPP
#define _SHARPEN_GENERICMAILPASER_HPP

#include "GenericMailHeader.hpp"   // IWYU pragma: export
#include "IMailParser.hpp"
#include <deque>
#include <limits>

namespace sharpen {
    class GenericMailParser : public sharpen::IMailParser {
    private:
        using Self = sharpen::GenericMailParser;

        enum class ParseStatus{
            Header,
            Content
        };

        std::uint32_t magic_;
        std::uint32_t maxContentSize_;
        std::size_t parsedSize_;
        sharpen::ByteBuffer header_;
        sharpen::ByteBuffer content_;
        std::deque<sharpen::Mail> completedMails_;

        ParseStatus GetStatus() const noexcept;

        virtual sharpen::Mail NviPopCompletedMail() noexcept override;

        virtual void NviParse(sharpen::ByteSlice slice) override;

    public:
        constexpr static std::uint32_t minMaxContentSize{4*1024};

        constexpr static std::uint32_t maxMaxContentSize{(std::numeric_limits<std::uint32_t>::max)()};

        constexpr static std::uint32_t defaultMaxContentSize{32*1024*1024};

        explicit GenericMailParser(std::uint32_t magic) noexcept;

        GenericMailParser(const Self &other);

        GenericMailParser(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual ~GenericMailParser() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual bool Completed() const noexcept override;

        void PrepareMaxContentSize(std::uint32_t maxContentSize) noexcept;
    };
}   // namespace sharpen

#endif