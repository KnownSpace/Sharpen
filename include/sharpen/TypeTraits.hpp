#pragma once
#ifndef _SHARPEN_TYPETRAITS_HPP
#define _SHARPEN_TYPETRAITS_HPP

#include <type_traits>

#include "TypeDef.hpp"

namespace sharpen
{
    template<bool _Value>
    struct BoolType
    {
        constexpr static bool Value = _Value;

        constexpr bool operator()() const noexcept
        {
            return _Value;
        }

        constexpr operator bool() const noexcept
        {
            return _Value;
        }
    };
    
    using TrueType = sharpen::BoolType<true>;

    using FalseType = sharpen::BoolType<false>;

    template<typename _Fn,typename ..._Args>
    struct InternalIsCallable
    {
    private:
        template<typename _Check = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        constexpr static sharpen::TrueType Test(int) noexcept
        {
            return sharpen::TrueType();
        }

        constexpr static sharpen::FalseType Test(...) noexcept
        {
            return sharpen::FalseType();
        }
    public:
        using Type = decltype(sharpen::InternalIsCallable<_Fn,_Args...>::Test(0));

        static constexpr bool Value = Type::Value;
    };

    template<typename _Fn,typename ..._Args>
    using IsCallable = typename sharpen::InternalIsCallable<_Fn,_Args...>::Type;

    template<typename ..._T>
    using TypeChecker = void;

    template<typename _Checker>
    struct ValidContainer
    {
    private:
        template<typename _Arg,typename _Check = decltype(std::declval<_Checker>()(std::declval<_Arg>()))>
        constexpr sharpen::TrueType Test(int) noexcept
        {
            return sharpen::TrueType{};
        }

        template<typename _Arg>
        constexpr sharpen::FalseType Test(...) noexcept
        {
            return sharpen::FalseType{};
        }
    public:

        template<typename _Arg>
        constexpr auto operator()(_Arg &&arg) noexcept -> decltype(Test<_Arg>(0))
        {
            return Test<_Arg>(0);
        }
    };

    template<typename _Check>
    constexpr sharpen::ValidContainer<_Check> InternalIsValid() noexcept
    {
        return sharpen::ValidContainer<_Check>();
    }

    template<typename _Check,typename _T>
    using IsValid = decltype(sharpen::InternalIsValid<_Check>()(std::declval<_T>()));

    struct MatchesContainer
    {
        template<template<class ...> class _Tmp,typename ..._T,typename _Check = _Tmp<_T...>>
        constexpr static sharpen::TrueType Matches(int) noexcept
        {
            return sharpen::TrueType();
        }

        template<template<class ...> class _Tmp,typename ..._T>
        constexpr static sharpen::FalseType Matches(...) noexcept
        {
            return sharpen::FalseType();
        }
    };
    
    template<template<class ...> class _Tmp,typename ..._T>
    using IsMatches = decltype(sharpen::MatchesContainer::Matches<_Tmp,_T...>(0));

    template<bool _Cond,typename _T = void>
    using EnableIf = typename std::enable_if<_Cond,_T>::type;

    template<bool _Cond,typename _TrueType,typename _FalseType>
    struct InternalTypeIfElse
    {
        using Type = _TrueType;
    };

    template<typename _TrueType,typename _FalseType>
    struct InternalTypeIfElse<false,_TrueType,_FalseType>
    {
        using Type = _FalseType;
    };
    
    template<bool _Cond,typename _TrueType,typename _FalseType>
    using EnableIfElse = typename sharpen::InternalTypeIfElse<_Cond,_TrueType,_FalseType>::Type;

    template<typename _T>
    using InternalIsCompletedType = auto(*)() -> decltype(sizeof(_T));

    template<typename _T>
    using IsCompletedType = sharpen::IsMatches<sharpen::InternalIsCompletedType,_T>;

    struct InternalEmptyTestBase
    {
        char flag_;
    };

    struct InternalEmptyType
    {};

    template<typename _T>
    struct InternalEmptyTest:public _T,public sharpen::InternalEmptyTestBase
    {};

    template<typename _T,sharpen::Size _Size>
    struct InternalIsEmptyType:sharpen::FalseType
    {};

    template<sharpen::Size _Size>
    struct InternalIsEmptyType<bool,_Size>:public sharpen::FalseType
    {};

    template<sharpen::Size _Size>
    struct InternalIsEmptyType<char,_Size>:public sharpen::FalseType
    {};

    template<sharpen::Size _Size>
    struct InternalIsEmptyType<unsigned char,_Size>:public sharpen::FalseType
    {};

    template<typename _T>
    struct InternalIsEmptyType<_T,sizeof(sharpen::InternalEmptyType)>:public sharpen::EnableIfElse<(sizeof(sharpen::InternalEmptyTest<_T>) == sizeof(sharpen::InternalEmptyTestBase)),sharpen::TrueType,sharpen::FalseType>
    {};
    
    template<typename _T,typename _Check = void>
    struct IsEmptyType:public sharpen::FalseType
    {};
    
    template<typename _T>
    struct IsEmptyType<_T,sharpen::EnableIf<sharpen::IsCompletedType<_T>::Value>>:public sharpen::InternalIsEmptyType<_T,sizeof(_T)>
    {};

    template<template<class> class _Matches,typename _Arg,typename ..._Args>
    struct InternalMultiMatches
    {
    public:
        static constexpr bool Value = sharpen::IsMatches<_Matches,_Arg>::Value;

        using Type = sharpen::BoolType<Value && sharpen::InternalMultiMatches<_T,_Args...>::Value>;
    };

    template<template<class> class _Matches,typename _Arg>
    struct InternalMultiMatches<_Matches,_Arg>
    {
    public:
        static constexpr bool Value = sharpen::IsMatches<_Matches,_Arg>::Value;

        using Type = sharpen::BoolType<Value>;
    };
    
    template<typename _T,typename _Arg,typename ..._Args>
    using MultiMatches = typename sharpen::InternalMultiMatches<_T,_Arg,_Args...>::Type;
}

#endif