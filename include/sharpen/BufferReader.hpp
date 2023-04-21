#pragma once
#ifndef _SHARPEN_BUFFERREADER_HPP
#define _SHARPEN_BUFFERREADER_HPP

#include <cassert>

#include "BinarySerializator.hpp"
#include "ByteBuffer.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class BufferReader : public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::BufferReader;

        std::size_t offset_;
        const sharpen::ByteBuffer *target_;

    public:
        explicit BufferReader(const sharpen::ByteBuffer &target)
            : offset_(0)
            , target_(&target)
        {
            assert(this->target_);
        }

        BufferReader(Self &&other) noexcept
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

        ~BufferReader() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline bool Readable() const noexcept
        {
            return this->target_;
        }

        inline const sharpen::ByteBuffer &Target() const noexcept
        {
            assert(this->target_);
            return *this->target_;
        }

        template<typename _T,
                 typename _Check = decltype(sharpen::BinarySerializator::LoadFrom(
                     std::declval<_T &>(), nullptr, 0))>
        inline void Read(_T &obj)
        {
            assert(this->target_);
            std::size_t sz{this->target_->GetSize() - this->offset_};
            if (!sz)
            {
                throw std::out_of_range{"index out of range"};
            }
            this->offset_ += sharpen::BinarySerializator::LoadFrom(
                obj, this->target_->Data() + this->offset_, sz);
        }

        inline void Read(char *data, std::size_t size)
        {
            assert(this->target_);
            std::size_t sz{this->target_->GetSize() - this->offset_};
            if (sz < size)
            {
                throw std::out_of_range{"index out of range"};
            }
            for (std::size_t i = 0; i != size; ++i)
            {
                data[i] = this->target_->Get(i + this->offset_);
            }
            this->offset_ += size;
        }

        template<typename _T,
                 typename _Check = decltype(
                     sharpen::BinarySerializator::LoadFrom(std::declval<_T &>(), nullptr, 0), _T{})>
        inline _T Read()
        {
            _T obj{};
            this->Read(obj);
            return obj;
        }

        inline std::size_t GetOffset() const noexcept
        {
            return this->offset_;
        }
    };
}   // namespace sharpen

#endif