#pragma once
#ifndef _SHARPEN_ITERATOROPS_HPP
#define _SHARPEN_ITERATOROPS_HPP

#include <type_traits>

#include "TypeDef.hpp"
#include "NoexceptIf.hpp"

namespace sharpen
{
    template<typename _Iterator>
    inline auto InternalIteratorForward(_Iterator ite,sharpen::Size n,int) SHARPEN_NOEXCEPT_IF(ite + n) -> decltype(ite + n)
    {
        return ite + n;
    }

    template<typename _Iterator>
    inline auto InternalIteratorForward(_Iterator ite,sharpen::Size n,...) SHARPEN_NOEXCEPT_IF(ite++) -> decltype(ite++)
    {
        while (n != 0)
        {
            ++ite;
            n -= 1;
        }
        return ite;
    }

    template<typename _Iterator>
    inline auto IteratorForward(_Iterator ite,sharpen::Size n) SHARPEN_NOEXCEPT_IF(sharpen::InternalIteratorForward(ite,n,0)) -> decltype(sharpen::InternalIteratorForward(ite,n,0))
    {
        return sharpen::InternalIteratorForward(ite,n,0);   
    }

    template<typename _Iterator>
    inline auto InternalIteratorBackward(_Iterator ite,sharpen::Size n,int) SHARPEN_NOEXCEPT_IF(ite - n) -> decltype(ite - n)
    {
        return ite - n;
    }

    template<typename _Iterator>
    inline auto InternalIteratorBackward(_Iterator ite,sharpen::Size n,...) SHARPEN_NOEXCEPT_IF(--ite) -> decltype(--ite)
    {
        while (n != 0)
        {
            --ite;
            n -= 1;
        }
        return ite;
    }

    template<typename _Iterator>
    inline auto IteratorBackward(_Iterator ite,sharpen::Size n) SHARPEN_NOEXCEPT_IF(sharpen::InternalIteratorBackward(ite,n,0)) -> decltype(sharpen::InternalIteratorBackward(ite,n,0))
    {
        return sharpen::InternalIteratorBackward(ite,n,0);   
    }

    template<typename _Iterator>
    inline auto InternalGetRangeSize(_Iterator begin,_Iterator end,int) SHARPEN_NOEXCEPT_IF(end - begin) -> decltype(static_cast<sharpen::Size>(end - begin))
    {
        return end - begin;
    }

    template<typename _Iterator,typename _HasForward = decltype(sharpen::IteratorForward(std::declval<_Iterator>(),0))>
    inline sharpen::Size InternalGetRangeSize(_Iterator begin,_Iterator end,...) SHARPEN_NOEXCEPT_IF(sharpen::IteratorForward(std::declval<_Iterator>(),0))
    {
        sharpen::Size size{0};
        while (begin != end)
        {
            ++size;
            ++begin;
        }
        return size;
    }

    template<typename _Iterator>
    inline auto GetRangeSize(_Iterator begin,_Iterator end) SHARPEN_NOEXCEPT_IF(sharpen::InternalGetRangeSize(begin,end,0)) -> decltype(sharpen::InternalGetRangeSize(begin,end,0))
    {
        return sharpen::InternalGetRangeSize(begin,end,0);
    }
}

#endif