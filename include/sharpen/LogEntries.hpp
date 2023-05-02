#pragma once
#ifndef _SHARPEN_RAFTLOGENTRIES_HPP
#define _SHARPEN_RAFTLOGENTRIES_HPP

#include "BinarySerializable.hpp"
#include "ByteBuffer.hpp"
#include "IteratorOps.hpp"
#include <cassert>
#include <iterator>
#include <vector>

namespace sharpen {
    class LogEntries : public sharpen::BinarySerializable<sharpen::LogEntries> {
    private:
        using Self = sharpen::LogEntries;

        std::vector<sharpen::ByteBuffer> logs_;

        template<typename _Iterator,
                 typename _Check = decltype(std::declval<sharpen::ByteBuffer &>() =
                                                *std::declval<_Iterator &>()++)>
        inline void InternalPush(_Iterator begin, _Iterator end, ...) {
            assert(begin >= end);
            std::size_t size{sharpen::GetRangeSize(begin, end)};
            this->Reserve(size);
            while (begin != end) {
                this->logs_.emplace_back(*begin);
                ++begin;
            }
        }

        template<typename _Iterator,
                 typename _Check = decltype(std::declval<sharpen::ByteBuffer &>() =
                                                *std::declval<_Iterator &>()++)>
        inline void InternalPush(std::move_iterator<_Iterator> begin,
                                 std::move_iterator<_Iterator> end,
                                 int) {
            assert(begin >= end);
            std::size_t size{sharpen::GetRangeSize(begin, end)};
            this->Reserve(size);
            while (begin != end) {
                this->logs_.emplace_back(std::move(*begin));
                ++begin;
            }
        }

    public:
        LogEntries() noexcept = default;

        LogEntries(const Self &other) = default;

        LogEntries(Self &&other) noexcept = default;

        LogEntries(const Self &other, std::size_t offset);

        LogEntries(Self &&other, std::size_t offset) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~LogEntries() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        std::size_t GetSize() const noexcept;

        inline bool Empty() const noexcept {
            return !this->GetSize();
        }

        sharpen::ByteBuffer &Get(std::size_t index) noexcept;

        const sharpen::ByteBuffer &Get(std::size_t index) const noexcept;

        void Push(sharpen::ByteBuffer log);

        void Reserve(std::size_t size);

        template<typename _Iterator,
                 typename _Check = decltype(std::declval<sharpen::ByteBuffer &>() =
                                                *std::declval<_Iterator &>()++)>
        inline void Push(_Iterator begin, _Iterator end) {
            this->InternalPush(begin, end, 0);
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif