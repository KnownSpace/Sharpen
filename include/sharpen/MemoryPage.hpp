#pragma once
#ifndef _SHARPEN_MEMORYPAGE_HPP
#define _SHARPEN_MEMORYPAGE_HPP

#include "AlignedAlloc.hpp"
#include "ByteSlice.hpp"
#include <exception>
#include <stdexcept>

namespace sharpen {
    class MemoryPage {
    private:
        using Self = sharpen::MemoryPage;
        using ConstPtr = const char *;

        static constexpr std::size_t pageSize_{4096};

        char *data_;
        std::size_t pageCount_;

    public:
        MemoryPage() noexcept;

        explicit MemoryPage(std::size_t pageCount);

        MemoryPage(const Self &other);

        MemoryPage(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~MemoryPage() noexcept;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void Free() noexcept;

        inline char *Data() noexcept {
            return this->data_;
        }

        inline const char *Data() const noexcept {
            return this->data_;
        }

        inline char &Get(std::size_t index) noexcept {
            assert(this->data_);
            return this->data_[index];
        }

        inline const char &Get(std::size_t index) const noexcept {
            assert(this->data_);
            return this->data_[index];
        }

        inline operator char *() noexcept {
            return this->Data();
        }

        inline operator ConstPtr() const noexcept {
            return this->Data();
        }

        inline char &operator*() noexcept {
            return this->Get(0);
        }

        inline const char &operator*() const noexcept {
            return this->Get(0);
        }

        inline char &operator[](std::size_t index) noexcept {
            return this->Get(index);
        }

        inline const char &operator[](std::size_t index) const noexcept {
            return this->Get(index);
        }

        inline std::size_t GetPageCount() const noexcept {
            return this->pageCount_;
        }

        inline std::size_t GetSize() const noexcept {
            return this->GetPageCount() * pageSize_;
        }

        bool Empty() const noexcept {
            return this->pageCount_;
        }

        sharpen::ByteSlice GetSlice(std::size_t index, std::size_t size) const;

        inline sharpen::ByteSlice GetSlice() const noexcept {
            return sharpen::ByteSlice{this->Data(), this->GetSize()};
        }
    };
}   // namespace sharpen

#endif