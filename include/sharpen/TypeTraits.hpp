#pragma once
#ifndef _SHARPEN_TYPETRAITS_HPP
#define _SHARPEN_TYPETRAITS_HPP

#include <type_traits>

namespace sharpen
{
    template<typename _Fn,typename ..._Args>
    struct IsCallable
    {
    private:
        struct FasleType;

        template<typename _U>
        static auto Test(int) -> decltype(std::declval<_Fn>()(std::declval<_Args>()...));

        static FasleType Test(...);
    public:
        static constexpr bool Value = std::is_same<FasleType,decltype(Test(1))>::value;
    };
}

#endif