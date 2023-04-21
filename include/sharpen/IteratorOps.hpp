#pragma once
#ifndef _SHARPEN_ITERATOROPS_HPP
#define _SHARPEN_ITERATOROPS_HPP

#include <type_traits>

#include "NoexceptIf.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen
{
    template<typename _Iterator>
    inline auto InternalIteratorForward(_Iterator ite, std::size_t n, int)
        SHARPEN_NOEXCEPT_IF(ite + n) -> decltype(ite + n)
    {
        return ite + n;
    }

    template<typename _Iterator>
    inline auto InternalIteratorForward(_Iterator ite, std::size_t n, ...)
        SHARPEN_NOEXCEPT_IF(ite++) -> decltype(ite++)
    {
        while (n != 0)
        {
            ++ite;
            n -= 1;
        }
        return ite;
    }

    template<typename _Iterator>
    inline auto IteratorForward(_Iterator ite, std::size_t n)
        SHARPEN_NOEXCEPT_IF(sharpen::InternalIteratorForward(ite, n, 0))
            -> decltype(sharpen::InternalIteratorForward(ite, n, 0))
    {
        return sharpen::InternalIteratorForward(ite, n, 0);
    }

    template<typename _Iterator>
    inline auto InternalIteratorBackward(_Iterator ite, std::size_t n, int)
        SHARPEN_NOEXCEPT_IF(ite - n) -> decltype(ite - n)
    {
        return ite - n;
    }

    template<typename _Iterator>
    inline auto InternalIteratorBackward(_Iterator ite, std::size_t n, ...)
        SHARPEN_NOEXCEPT_IF(--ite) -> decltype(--ite)
    {
        while (n != 0)
        {
            --ite;
            n -= 1;
        }
        return ite;
    }

    template<typename _Iterator>
    inline auto IteratorBackward(_Iterator ite, std::size_t n)
        SHARPEN_NOEXCEPT_IF(sharpen::InternalIteratorBackward(ite, n, 0))
            -> decltype(sharpen::InternalIteratorBackward(ite, n, 0))
    {
        return sharpen::InternalIteratorBackward(ite, n, 0);
    }

    template<typename _Iterator>
    inline auto InternalGetRangeSize(_Iterator begin, _Iterator end, int)
        SHARPEN_NOEXCEPT_IF(end - begin) -> decltype(static_cast<std::size_t>(end - begin))
    {
        return end - begin;
    }

    template<typename _Iterator,
             typename _HasForward = decltype(sharpen::IteratorForward(std::declval<_Iterator>(),
                                                                      0))>
    inline std::size_t InternalGetRangeSize(_Iterator begin, _Iterator end, ...)
        SHARPEN_NOEXCEPT_IF(sharpen::IteratorForward(std::declval<_Iterator>(), 0))
    {
        std::size_t size{0};
        while (begin != end)
        {
            ++size;
            ++begin;
        }
        return size;
    }

    template<typename _Iterator>
    inline auto GetRangeSize(_Iterator begin, _Iterator end)
        SHARPEN_NOEXCEPT_IF(sharpen::InternalGetRangeSize(begin, end, 0))
            -> decltype(sharpen::InternalGetRangeSize(begin, end, 0))
    {
        return sharpen::InternalGetRangeSize(begin, end, 0);
    }

    template<typename _Iterator,
             typename _Finder,
             typename _Check = decltype(false ==
                                        std::declval<_Finder>()(*std::declval<_Iterator>()))>
    inline auto Find(_Iterator begin, _Iterator end, _Finder &&finder) -> decltype(++begin)
    {
        while (begin != end)
        {
            if (finder(*begin))
            {
                break;
            }
            ++begin;
        }
        return begin;
    }

    template<typename _Container>
    inline auto InternalBegin(_Container &&container, int) -> decltype(container.Begin())
    {
        return container.Begin();
    }

    template<typename _Container>
    inline auto InternalBegin(_Container &&container, ...) -> decltype(std::begin(container))
    {
        return std::begin(container);
    }

    template<typename _Container>
    inline auto Begin(_Container &&container) -> decltype(sharpen::InternalBegin(container, 0))
    {
        return sharpen::InternalBegin(container, 0);
    }

    template<typename _Container>
    inline auto InternalEnd(_Container &&container, int) -> decltype(container.End())
    {
        return container.End();
    }

    template<typename _Container>
    inline auto InternalEnd(_Container &&container, ...) -> decltype(std::end(container))
    {
        return std::end(container);
    }

    template<typename _Container>
    inline auto End(_Container &&container) -> decltype(sharpen::InternalEnd(container, 0))
    {
        return sharpen::InternalEnd(container, 0);
    }

    template<typename _Iterator>
    class IteratorRanage
    {
    private:
        using Self = sharpen::IteratorRanage<_Iterator>;

        _Iterator begin_;
        _Iterator end_;

    public:
        IteratorRanage(_Iterator begin, _Iterator end)
            : begin_(begin)
            , end_(end)
        {
        }

        IteratorRanage(const Self &other) = default;

        IteratorRanage(Self &&other) noexcept = default;

        ~IteratorRanage() noexcept = default;

        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(*this, tmp);
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if (this != std::addressof(other))
            {
                this->begin_ = std::move(other.begin_);
                this->end_ = std::move(other.end_);
            }
            return *this;
        }

        inline _Iterator GetBegin() const noexcept
        {
            return this->begin_;
        }

        inline _Iterator GetEnd() const noexcept
        {
            return this->end_;
        }

        inline std::size_t GetSize() const noexcept
        {
            return sharpen::GetRangeSize(this->begin_, this->end_);
        }

        inline bool Empty() const noexcept
        {
            return !this->GetSize();
        }
    };
}   // namespace sharpen

#endif