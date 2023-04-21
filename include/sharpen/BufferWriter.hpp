#pragma once
#ifndef _SHARPEN_BUFFERWRITER_HPP
#define _SHARPEN_BUFFERWRITER_HPP

#include <cassert>

#include "BinarySerializator.hpp"
#include "ByteBuffer.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class BufferWriter : public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::BufferWriter;

        std::size_t offset_;
        sharpen::ByteBuffer *target_;

    public:
        explicit BufferWriter(sharpen::ByteBuffer &target)
            : offset_(0)
            , target_(&target)
        {
            assert(this->target_);
        }

        BufferWriter(Self &&other) noexcept
            : offset_(other.offset_)
            , target_(other.target_)
        {
            other.offset_ = 0;
            other.target_ = nullptr;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if (this != std::addressof(other))
            {
                this->offset_ = other.offset_;
                this->target_ = other.target_;
                other.offset_ = 0;
                other.target_ = nullptr;
            }
            return *this;
        }

        ~BufferWriter() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline bool Writable() const noexcept
        {
            return this->target_;
        }

        inline sharpen::ByteBuffer &Target() noexcept
        {
            assert(this->target_);
            return *this->target_;
        }

        inline const sharpen::ByteBuffer &Target() const noexcept
        {
            assert(this->target_);
            return *this->target_;
        }

        template<typename _T,
                 typename _Check =
                     decltype(sharpen::BinarySerializator::UnsafeStoreTo(std::declval<const _T &>(),
                                                                         nullptr),
                              sharpen::BinarySerializator::ComputeSize(std::declval<const _T &>()))>
        inline void Write(const _T &obj)
        {
            assert(this->target_);
            std::size_t sz{sharpen::BinarySerializator::ComputeSize(obj)};
            if (this->target_->GetSize() < this->offset_ + sz)
            {
                this->target_->ExtendTo(this->offset_ + sz);
            }
            this->offset_ += sharpen::BinarySerializator::UnsafeStoreTo(
                obj, this->target_->Data() + this->offset_);
        }

        inline void Write(const char *data, std::size_t size)
        {
            assert(this->target_);
            if (this->target_->GetSize() < this->offset_ + size)
            {
                this->target_->ExtendTo(this->offset_ + size);
            }
            for (size_t i = 0; i != size; ++i)
            {
                this->target_->Get(this->offset_ + i) = data[i];
            }
            this->offset_ += size;
        }

        inline std::size_t GetLength() const noexcept
        {
            return this->offset_;
        }

        inline void Resize(std::size_t size)
        {
            assert(this->target_);
            this->target_->ExtendTo(size);
        }
    };
}   // namespace sharpen

#endif