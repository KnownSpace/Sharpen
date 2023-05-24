#pragma once
#ifndef _SHARPEN_GENERICMAILPASER_HPP
#define _SHARPEN_GENERICMAILPASER_HPP

#include "GenericMailHeader.hpp"   // IWYU pragma: export
#include "IMailParser.hpp"
#include <deque>

namespace sharpen {
    class GenericMailPaser : public sharpen::IMailParser {
    private:
        using Self = GenericMailPaser;

        std::uint32_t magic_;
        std::size_t parsedSize_;
        sharpen::ByteBuffer header_;
        sharpen::ByteBuffer content_;
        std::deque<sharpen::Mail> completedMails_;

        virtual sharpen::Mail NviPopCompletedMail() noexcept override;

        virtual void NviParse(sharpen::ByteSlice slice) override;

    public:
        GenericMailPaser(std::uint32_t magic) noexcept;

        GenericMailPaser(const Self &other);

        GenericMailPaser(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual ~GenericMailPaser() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual bool Completed() const noexcept override;
    };
}   // namespace sharpen

#endif