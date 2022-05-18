#pragma once
#ifndef _SHARPEN_FUNCTIONALOPS_HPP
#define _SHARPEN_FUNCTIONALOPS_HPP

#include "CompilerInfo.hpp"

#ifdef SHARPEN_COMPILER_MSVC
#define SHARPEN_THIS_CALL __thiscall
#else
#define SHARPEN_THIS_CALL
#endif

namespace sharpen
{

    template<typename _Class,typename _Ret,typename ..._Args>
    using TrivialFunctionPtr = _Ret(SHARPEN_THIS_CALL *)(_Class*,_Args...);

    template<typename _Class,typename _Ret,typename ..._Args>
    union InternalMemberFunctionConvertor
    {
        _Ret(_Class::*member_)(_Args...);
        sharpen::TrivialFunctionPtr<_Class,_Ret,_Args...> normal_;
    };

    template<typename _Class,typename _Ret,typename ..._Args>
    union InternalConstMemberFunctionConvertor
    {
        _Ret(_Class::*member_)(_Args...) const;
        sharpen::TrivialFunctionPtr<const _Class,_Ret,_Args...> normal_;
    };

    template<typename _Class,typename _Ret,typename ..._Args>
    inline auto InternalConvertToFunctionPtr(_Ret(_Class::*fp)(_Args...) const,int) -> sharpen::TrivialFunctionPtr<const _Class,_Ret,_Args...>
    {
        sharpen::InternalConstMemberFunctionConvertor<_Class,_Ret,_Args...> u;
        u.member_ = fp;
        return u.normal_;
    }

    template<typename _Class,typename _Ret,typename ..._Args>
    inline auto InternalConvertToFunctionPtr(_Ret(_Class::*fp)(_Args...),...) -> sharpen::TrivialFunctionPtr<_Class,_Ret,_Args...>
    {
        sharpen::InternalMemberFunctionConvertor<_Class,_Ret,_Args...> u;
        u.member_ = fp;
        return u.normal_;
    }

    template<typename _Fn>
    inline auto TrivialFunction(_Fn fn) -> decltype(sharpen::InternalConvertToFunctionPtr(fn,0))
    {
        return sharpen::InternalConvertToFunctionPtr(fn,0);
    }
}

#undef SHARPEN_THIS_CALL
#endif