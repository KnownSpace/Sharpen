#pragma once
#ifndef _SHARPEN_BYTEVECTOR_HPP
#define _SHARPEN_BYTEVECTOR_HPP

#include "IteratorOps.hpp"
#include "PointerIterator.hpp"
#include "ReversePointerIterator.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace sharpen {
    struct ByteVectorStruct {
        char *data_;
        std::size_t cap_;
    };

    template<std::size_t _InlineSize>
    union ByteVectorUnion {
        sharpen::ByteVectorStruct external_;
        char inline_[_InlineSize];
    };

    class ByteVector {
    private:
        using Self = ByteVector;

        static constexpr std::size_t inlineSize_{24};
        static constexpr std::size_t blobSize_{1 * 1024 * 1024};

        static_assert(inlineSize_ > sizeof(sharpen::ByteVectorStruct), "Inline size too small");

        std::size_t size_;
        sharpen::ByteVectorUnion<sharpen::ByteVector::inlineSize_> rawVector_;

        inline static bool InlineBuffer(std::size_t size) noexcept {
            return size <= inlineSize_;
        }

        inline bool InlineBuffer() const noexcept {
            return this->InlineBuffer(this->size_);
        }

        static std::size_t ComputeHeapSize(std::size_t size) noexcept;

        void MoveFrom(Self &&other) noexcept;

        bool CheckPointer(const char *p);

        static void *Alloc(std::size_t size) noexcept;

        static void Free(void *p) noexcept;

    public:
        using Iterator = sharpen::PointerIterator<char>;
        using ConstIterator = sharpen::PointerIterator<const char>;
        using ReverseIterator = sharpen::ReversePointerIterator<char>;
        using ConstReverseIterator = sharpen::ReversePointerIterator<const char>;

        static constexpr std::size_t inlineSize{inlineSize_};

        inline static constexpr std::size_t GetInlineSize() noexcept {
            return inlineSize;
        }

        ByteVector() noexcept
            : size_(0)
            , rawVector_() {
        }

        explicit ByteVector(std::size_t size);

        ByteVector(const Self &other);

        ByteVector(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~ByteVector() noexcept {
            this->Clear();
        }

        inline const Self &Const() const noexcept {
            return *this;
        }

        char *Data() noexcept;

        const char *Data() const noexcept;

        char &Get(std::size_t index);

        char Get(std::size_t index) const;

        inline std::size_t GetSize() const noexcept {
            return this->size_;
        }

        void Clear() noexcept;

        void Resize(std::size_t newSize, char defaultVal);

        inline void Resize(std::size_t newSize) {
            this->Resize(newSize, 0);
        }

        template<
            typename _Iterator,
            typename _Check = decltype(std::declval<char &>() = *std::declval<_Iterator &>()++)>
        inline void Append(_Iterator begin, _Iterator end) {
            std::size_t size{sharpen::GetRangeSize(begin, end)};
            std::size_t oldSize{this->GetSize()};
            std::size_t newSize{oldSize + size};
            this->Resize(newSize);
            char *buf = this->Data();
            while (oldSize != newSize) {
                buf[oldSize++] = *begin++;
            }
        }

        inline void PushBack(char c) {
            this->Append(&c, &c + 1);
        }

        void Erase(std::size_t begin, std::size_t end) noexcept;

        inline void Erase(std::size_t where) noexcept {
            return this->Erase(where, where + 1);
        }

        void Erase(Iterator where);

        void Erase(Iterator begin, Iterator end);

        void Erase(ConstIterator where);

        void Erase(ConstIterator begin, ConstIterator end);

        inline void PopBack() {
            if (this->size_) {
                this->Erase(this->size_ - 1);
            }
        }

        inline bool Empty() const noexcept {
            return !this->size_;
        }

        Iterator Begin() noexcept;

        ConstIterator Begin() const noexcept;

        Iterator End() noexcept;

        ConstIterator End() const noexcept;

        ReverseIterator ReverseBegin() noexcept;

        ConstReverseIterator ReverseBegin() const noexcept;

        ReverseIterator ReverseEnd() noexcept;

        ConstReverseIterator ReverseEnd() const noexcept;

        char &operator[](std::size_t index) noexcept {
            return this->Data()[index];
        }

        char operator[](std::size_t index) const noexcept {
            return this->Data()[index];
        }

        inline char &Front() {
            return this->Get(0);
        }

        inline char Front() const noexcept {
            return this->Get(0);
        }

        inline char &Back() {
            if (this->Empty()) {
                throw std::out_of_range("index out of range");
            }
            return this->Get(this->GetSize() - 1);
        }

        inline char Back() const {
            if (this->Empty()) {
                throw std::out_of_range("index out of range");
            }
            return this->Get(this->GetSize() - 1);
        }
    };
}   // namespace sharpen

#endif