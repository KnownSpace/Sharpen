#pragma once
#ifndef _SHARPEN_ITERATORTEMPLATE_HPP
#define _SHARPEN_ITERATORTEMPLATE_HPP

#include <cstddef>

namespace sharpen
{
    template<typename _IteratorCategory,
             typename _ValueType,
             typename _DistanceType,
             typename _PointerType,
             typename _ReferenceType>
    struct IteratorTemplate
    {
        using iterator_category = _IteratorCategory;
        using value_type = _ValueType;
        using difference_type = _DistanceType;
        using pointer = _PointerType;
        using reference = _ReferenceType;
    };

    template<typename _IteratorCategory, typename _T>
    using DefaultIteratorTemplate =
        sharpen::IteratorTemplate<_IteratorCategory, _T, std::ptrdiff_t, _T *, _T &>;
}   // namespace sharpen

#endif