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
        static constexpr bool Value = std::is_same<FasleType,decltype(Test(0))>::value;
    };

    template<typename ..._T>
    using TypeChecker = void;

    template<typename _Fn>
    struct ValidContainer
    {
    private:
        template<typename _Arg,typename _Check = decltype(std::declval<_Fn>()(std::declval<_Arg>()))>
        constexpr std::true_type Test(int)
        {
            return std::true_type{};
        }

        template<typename _Arg>
        constexpr std::false_type Test(...)
        {
            return std::false_type{};
        }
    public:

        template<typename _Arg>
        constexpr auto operator()(_Arg &&arg)
        {
            return Test<_Arg>(0);
        }
    };

    template<typename _Check>
    constexpr auto Valid(_Check &&check)
    {
        return sharpen::ValidContainer<_Check>();
    }
}

#endif