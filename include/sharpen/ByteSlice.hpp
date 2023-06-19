#pragma once
#ifndef _SHARPEN_BYTESLICE_HPP
#define _SHARPEN_BYTESLICE_HPP

#include "BufferOps.hpp"
#include "PointerIterator.hpp"
#include "ReversePointerIterator.hpp"
#include "TypeTraits.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <utility>

namespace sharpen {
    class ByteSlice {
    private:
        using Self = sharpen::ByteSlice;

        const char *data_;
        std::size_t size_;

    public:
        using ConstIterator = sharpen::PointerIterator<const char>;
        using ConstReverseIterator = sharpen::ReversePointerIterator<const char>;

        constexpr ByteSlice() noexcept
            : data_(nullptr)
            , size_(0) {
        }

        constexpr ByteSlice(const char *data, std::size_t size) noexcept
            : data_(data)
            , size_(size) {
        }

        ByteSlice(const Self &other) noexcept = default;

        ByteSlice(Self &&other) noexcept
            : data_(other.data_)
            , size_(other.size_) {
            other.data_ = nullptr;
            other.size_ = 0;
        }

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~ByteSlice() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        char Get(std::size_t index) const;

        inline const char *Data() const noexcept {
            return this->data_;
        }

        inline std::size_t GetSize() const noexcept {
            return this->size_;
        }

        inline bool Empty() const noexcept {
            return !this->size_;
        }

        inline operator bool() const noexcept {
            return this->size_;
        }

        inline char operator[](std::size_t index) const {
            return this->Get(index);
        }

        std::int32_t CompareWith(const Self &other) const noexcept;

        inline bool operator==(const Self &other) const noexcept {
            return this->CompareWith(other) == 0;
        }

        inline bool operator!=(const Self &other) const noexcept {
            return this->CompareWith(other) != 0;
        }

        inline bool operator<(const Self &other) const noexcept {
            return this->CompareWith(other) < 0;
        }

        inline bool operator>(const Self &other) const noexcept {
            return this->CompareWith(other) > 0;
        }

        inline bool operator>=(const Self &other) const noexcept {
            return this->CompareWith(other) >= 0;
        }

        inline bool operator<=(const Self &other) const noexcept {
            return this->CompareWith(other) <= 0;
        }

        inline std::uint32_t GetHashCode32() const noexcept {
            return sharpen::BufferHash32(this->Data(), this->GetSize());
        }

        inline std::uint64_t GetHashCode64() const noexcept {
            return sharpen::BufferHash64(this->Data(), this->GetSize());
        }

        inline std::size_t GetHashCode() const noexcept {
            return sharpen::BufferHash(this->Data(), this->GetSize());
        }

        inline std::uint32_t Adler32() const noexcept {
            return sharpen::Adler32(this->Data(), this->GetSize());
        }

        inline std::uint16_t Crc16() const noexcept {
            return sharpen::Crc16(this->Data(), this->GetSize());
        }

        inline std::uint32_t Crc32() const noexcept {
            return sharpen::Crc32(this->Data(),this->GetSize());
        }

        inline ConstIterator Begin() const noexcept {
            return ConstIterator{this->Data()};
        }

        inline ConstIterator End() const noexcept {
            return ConstIterator{this->Data() + this->GetSize()};
        }

        inline ConstReverseIterator ReverseBegin() const noexcept {
            const char *p = this->Data();
            if (p) {
                assert(this->GetSize());
                p += this->GetSize() - 1;
            }
            return ConstReverseIterator{p};
        }

        inline ConstReverseIterator ReverseEnd() const noexcept {
            return ConstReverseIterator{this->Data()};
        }

        template<typename _Iterator,
                 typename _Check = decltype(std::declval<char &>() ==
                                            *std::declval<_Iterator &>()++)>
        inline ConstIterator Search(const _Iterator begin, const _Iterator end) const {
            return std::search(this->Begin(), this->End(), begin, end);
        }

        ConstIterator Find(char e) const noexcept;

        ConstReverseIterator ReverseFind(char e) const noexcept;

        template<typename _T,
                 typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline _T &As() noexcept {
            assert(this->GetSize() == sizeof(_T));
            return *reinterpret_cast<_T *>(this->Data());
        }

        template<typename _T,
                 typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline const _T &As() const noexcept {
            assert(this->GetSize() == sizeof(_T));
            return *reinterpret_cast<const _T *>(this->Data());
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t StoreTo(char *data, std::size_t size) const;

        Self Sub(std::size_t begin, std::size_t end) noexcept;

        Self Sub(std::size_t begin) noexcept;
    };
}   // namespace sharpen

namespace std {
    template<>
    struct hash<sharpen::ByteSlice> {
        inline std::size_t operator()(const sharpen::ByteSlice &slice) const noexcept {
            return slice.GetHashCode();
        }
    };
}   // namespace std

#endif