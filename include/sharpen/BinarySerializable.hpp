#pragma once
#ifndef _SHARPEN_BINARYSERIALIZABLE_HPP
#define _SHARPEN_BINARYSERIALIZABLE_HPP

#include "BinarySerializator.hpp"

namespace sharpen {
    template<typename _Object>
    class BinarySerializable {
    private:
        using Self = sharpen::BinarySerializable<_Object>;

    protected:
        using Serializator = sharpen::BinarySerializator;
        using Helper = Self::Serializator;

    public:
        BinarySerializable() noexcept = default;

        BinarySerializable(const Self &other) noexcept = default;

        BinarySerializable(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        ~BinarySerializable() noexcept = default;

        std::size_t ComputeSize() const noexcept {
            return Serializator::ComputeSize(*static_cast<const _Object *>(this));
        }

        std::size_t LoadFrom(const char *data, std::size_t size) {
            return Serializator::LoadFrom(*static_cast<_Object *>(this), data, size);
        }

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf, std::size_t offset) {
            return Serializator::LoadFrom(*static_cast<_Object *>(this), buf, offset);
        }

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf) {
            return Serializator::LoadFrom(*static_cast<_Object *>(this), buf, 0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept {
            return Serializator::UnsafeStoreTo(*static_cast<_Object *>(this), data);
        }

        std::size_t StoreTo(char *data, std::size_t size) const {
            return Serializator::StoreTo(*static_cast<const _Object *>(this), data, size);
        }

        std::size_t StoreTo(sharpen::ByteBuffer &buf, std::size_t offset) const {
            return Serializator::StoreTo(*static_cast<const _Object *>(this), buf, offset);
        }

        std::size_t StoreTo(sharpen::ByteBuffer &buf) const {
            return Serializator::StoreTo(*static_cast<const _Object *>(this), buf);
        }

        const sharpen::BinarySerializable<_Object> &Serialize() const noexcept {
            return *this;
        }

        sharpen::BinarySerializable<_Object> &Unserialize() noexcept {
            return *this;
        }

        /*
        Copy Me

        std::size_t ComputeSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t LoadFrom(const char *data,std::size_t size);

        */
    };
}   // namespace sharpen

#endif