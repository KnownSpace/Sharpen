#pragma once
#ifndef _SHARPEN_IMAILPARSER_HPP
#define _SHARPEN_IMAILPARSER_HPP

#include <memory>

#include "ByteSlice.hpp"
#include "Mail.hpp"
#include "MailParseError.hpp"

namespace sharpen
{
    class IMailParser
    {
    private:
        using Self = sharpen::IMailParser;

    protected:
        virtual sharpen::Mail NviPopCompletedMail() noexcept = 0;

        virtual void NviParse(sharpen::ByteSlice slice) = 0;

    public:
        IMailParser() noexcept = default;

        IMailParser(const Self &other) noexcept = default;

        IMailParser(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IMailParser() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void Parse(sharpen::ByteSlice slice)
        {
            if (!slice.Empty())
            {
                this->NviParse(slice);
            }
        }

        inline void Parse(const sharpen::ByteBuffer &buffer, std::size_t offset, std::size_t size)
        {
            this->NviParse(buffer.GetSlice(offset, size));
        }

        inline void Parse(const char *data, std::size_t size)
        {
            sharpen::ByteSlice slice{data, size};
            this->NviParse(slice);
        }

        inline void Parse(const sharpen::ByteBuffer &buffer)
        {
            this->NviParse(buffer.GetSlice());
        }

        inline void Parse(const sharpen::ByteBuffer &buffer, std::size_t offset)
        {
            assert(buffer.GetSize() > offset);
            std::size_t size{buffer.GetSize() - offset};
            this->Parse(buffer, offset, size);
        }

        inline sharpen::Mail PopCompletedMail()
        {
            if (!this->Completed())
            {
                throw sharpen::MailParseError{"Parse not complete"};
            }
            return this->NviPopCompletedMail();
        }

        virtual bool Completed() const noexcept = 0;
    };
}   // namespace sharpen

#endif