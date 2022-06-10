#pragma once
#ifndef _SHARPEN_TYPETRAITS_HPP
#define _SHARPEN_TYPETRAITS_HPP

#include <type_traits>
#include <functional>
#include <cstdint>
#include <cstddef>

namespace sharpen
{
    template<typename _Type,_Type _Value>
    struct ConstValue
    {
    private:
        using Self = sharpen::ConstValue<_Type,_Value>;

    public:
        constexpr static _Type Value{_Value};

        constexpr _Type operator()() const noexcept
        {
            return Self::Value;
        }

        constexpr operator _Type() const noexcept
        {
            return Self::Value;
        }
    };

    template<bool _Value>
    using BoolType = sharpen::ConstValue<bool,_Value>;
    
    using TrueType = sharpen::BoolType<true>;

    using FalseType = sharpen::BoolType<false>;

    template<std::size_t _Size>
    struct InternalIntType
    {
        using Type = std::int32_t;
    };

    template<>
    struct InternalIntType<8>
    {
        using Type = std::int8_t;
    };

    template<>
    struct InternalIntType<16>
    {
        using Type = std::int16_t;
    };

    template<>
    struct InternalIntType<32>
    {
        using Type = std::int32_t;
    };

    template<>
    struct InternalIntType<64>
    {
        using Type = std::int64_t;
    };

    template<std::size_t _Size>
    using IntType = typename sharpen::InternalIntType<_Size>::Type;

    template<std::size_t _Size>
    struct InternalUintType
    {
        using Type = std::uint32_t;
    };

    template<>
    struct InternalUintType<8>
    {
        using Type = std::uint8_t;
    };

    template<>
    struct InternalUintType<16>
    {
        using Type = std::uint16_t;
    };

    template<>
    struct InternalUintType<32>
    {
        using Type = std::uint32_t;
    };

    template<>
    struct InternalUintType<64>
    {
        using Type = std::uint64_t;
    };

    template<std::size_t _Size>
    using UintType = typename sharpen::InternalUintType<_Size>::Type;

    template<typename _Fn,typename ..._Args>
    struct InternalIsCallable
    {
    private:
        class TestFalse;

        using Self = sharpen::InternalIsCallable<_Fn,_Args...>;

        template<typename _U>
        constexpr static auto Test(int) noexcept -> decltype(std::declval<_U>()(std::declval<_Args>()...));

        template<typename _U>
        constexpr static TestFalse Test(...) noexcept;
    public:
        using Type = sharpen::BoolType<!std::is_same<decltype(Self::Test<_Fn>(0)),TestFalse>::value>;
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

    template<typename _T,std::size_t _Size>
    struct InternalIsEmptyType:sharpen::FalseType
    {};

    template<std::size_t _Size>
    struct InternalIsEmptyType<bool,_Size>:public sharpen::FalseType
    {};

    template<std::size_t _Size>
    struct InternalIsEmptyType<char,_Size>:public sharpen::FalseType
    {};

    template<std::size_t _Size>
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

        using Type = sharpen::BoolType<Value && sharpen::InternalMultiMatches<_Matches,_Args...>::Value>;
    };

    template<template<class> class _Matches,typename _Arg>
    struct InternalMultiMatches<_Matches,_Arg>
    {
    public:
        static constexpr bool Value = sharpen::IsMatches<_Matches,_Arg>::Value;

        using Type = sharpen::BoolType<Value>;
    };
    
    template<template<class> class _Matches,typename _Arg,typename ..._Args>
    using MultiMatches = typename sharpen::InternalMultiMatches<_Matches,_Arg,_Args...>::Type;

    template<typename _Int,_Int _First,_Int _Second,_Int ..._Values>
    struct MaxValue
    {
        static constexpr _Int Value = sharpen::MaxValue<_Int,(_First > _Second) ? _First:_Second,_Values...>::Value;
    };

    template<typename _Int,_Int _First,_Int _Second>
    struct MaxValue<_Int,_First,_Second>
    {
        static constexpr _Int Value = _First > _Second ? _First:_Second;
    };
    
    template<typename ..._Types>
    struct TypeList;

    template<>
    struct TypeList<>
    {
        template<typename _U>
        using PushBack = sharpen::TypeList<_U>;
        
        template<typename _U>
        using PushFront = sharpen::TypeList<_U>;

        template<typename _U>
        struct Find
        {
            constexpr static std::size_t Index = 0;
        };

        template<typename _U>        
        using Contain = sharpen::BoolType<false>;

        constexpr static std::size_t Size = 0;

        using First = void;
    };

    template<typename _TL,std::size_t _Index>
    struct InternalTypeListAt
    {
        using Type = typename sharpen::InternalTypeListAt<typename _TL::SubList,_Index - 1>::Type;
    };

    template<typename _TL>
    struct InternalTypeListAt<_TL,0>
    {
        using Type = typename _TL::First;
    };

    template<typename _TL,typename _U>
    struct InternalTypeListPushFront;

    template<typename _U,typename ..._Types>
    struct InternalTypeListPushFront<sharpen::TypeList<_Types...>,_U>
    {
        using Type = sharpen::TypeList<_U,_Types...>;
    };
     
    template<typename _TL,typename _U>
    struct InternalTypeListPushBack;

    template<typename _U,typename ..._Types>
    struct InternalTypeListPushBack<sharpen::TypeList<_Types...>,_U>
    {
        using Type = sharpen::TypeList<_Types...,_U>;
    };

    template<typename _TL,std::size_t _Index>
    struct InternalTypeListErase
    {
        using Sub = typename sharpen::InternalTypeListErase<typename _TL::SubList,(_Index - 1)>::Type;
        using Type = typename sharpen::InternalTypeListPushFront<Sub,typename _TL::First>;
    };
        
    template<typename _TL>
    struct InternalTypeListErase<_TL,0>
    {
        using Type = typename _TL::SubList;
    };

    template<typename _TL,std::size_t _Index,typename _U,std::size_t _Size>
    struct InternalTypeListInsert
    {
        using Sub = typename sharpen::InternalTypeListInsert<typename _TL::SubList,_Index - 1,_U,_TL::SubList::Size>::Type;
        using Type = typename sharpen::InternalTypeListPushFront<Sub,typename _TL::First>::Type;
    };
        
    template<typename _TL,typename _U,std::size_t _Size>
    struct InternalTypeListInsert<_TL,0,_U,_Size>
    {
        using Type = typename sharpen::InternalTypeListPushFront<_TL,_U>::Type;
    };

    template<typename _TL,typename _U,std::size_t _Size>
    struct InternalTypeListInsert<_TL,_Size,_U,_Size>
    {
        using Type = typename sharpen::InternalTypeListPushBack<_TL,_U>::Type;
    };

    template<typename _TL,typename _U,typename _First>
    struct InternalTypeListFind
    {
        using SubList = typename _TL::SubList;
        using Sub = typename sharpen::InternalTypeListFind<SubList,_U,typename SubList::First>;
        constexpr static std::size_t Index = 1 + Sub::Index;
    };

    template<typename _U,typename _First>
    struct InternalTypeListFind<sharpen::TypeList<>,_U,_First>
    {
        constexpr static std::size_t Index = 0;
    };
    
        
    template<typename _TL,typename _First>
    struct InternalTypeListFind<_TL,_First,_First>
    {
       constexpr static std::size_t Index = 0;
    };

    template<typename _T,typename ..._Types>
    struct TypeList<_T,_Types...>
    {
        using Self = sharpen::TypeList<_T,_Types...>;
        using First = _T;
        using SubList = sharpen::TypeList<_Types...>;

        constexpr static std::size_t Size = sizeof...(_Types) + 1;

        template<std::size_t _Index>
        using At = typename sharpen::InternalTypeListAt<Self,_Index>::Type;

        template<typename _U>
        using PushBack = sharpen::TypeList<_T,_Types...,_U>;
        
        template<typename _U>
        using PushFront = sharpen::TypeList<_U,_T,_Types...>;
        
        template<std::size_t _Index>
        using Erase = typename sharpen::InternalTypeListErase<Self,_Index>::Type;

        template<std::size_t _Index,typename _U>
        using Insert = typename InternalTypeListInsert<Self,_Index,_U,Size>::Type;

        template<typename _U>
        struct Find
        {
            constexpr static std::size_t Index = sharpen::InternalTypeListFind<Self,_U,_T>::Index;
        };

        template<typename _U>        
        using Contain = sharpen::BoolType<(Find<_U>::Index != Size)>;

        template<typename _U>
        using Remove = Erase<Find<_U>::Index>;

    };

    template<typename _Fn,typename ..._Args>
    using InternalIsCompletedBindable = auto(*)()->decltype(std::bind(std::declval<_Fn>(),std::declval<_Args>()...)());

    template<typename _Fn,typename ..._Args>
    using IsCompletedBindable = sharpen::IsMatches<sharpen::InternalIsCompletedBindable,_Fn,_Args...>;

    template<typename _R,typename _Fn,typename ..._Args>
    using InternalIsCallableReturned = auto(*)()->sharpen::EnableIf<std::is_same<_R,decltype(std::declval<_Fn>()(std::declval<_Args>()...))>::value>;

    template<typename _R,typename _Fn,typename ..._Args>
    using IsCallableReturned = sharpen::IsMatches<sharpen::InternalIsCallableReturned,_R,_Fn,_Args...>;

    template<typename _R,typename _Fn,typename ..._Args>
    using InternalIsCompletedBindableReturned = auto(*)()->sharpen::EnableIf<std::is_same<_R,decltype(std::bind(std::declval<_Fn>(),std::declval<_Args>()...)())>::value>;

    template<typename _R,typename _Fn,typename ..._Args>
    using IsCompletedBindableReturned = sharpen::IsMatches<sharpen::InternalIsCompletedBindableReturned,_R,_Fn,_Args...>;
}
#endif