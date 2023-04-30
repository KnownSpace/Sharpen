#pragma once
#ifndef _SHARPEN_FUNCTIONALOPS_HPP
#define _SHARPEN_FUNCTIONALOPS_HPP

#include "CompilerInfo.hpp"
#include <cassert>
#include <type_traits>
#include <utility>

#ifdef SHARPEN_COMPILER_MSVC
#define SHARPEN_THIS_CALL __thiscall
#else
#define SHARPEN_THIS_CALL
#endif

namespace sharpen
{

    template<typename _Class, typename _Ret, typename... _Args>
    using TrivialFunctionPtr = _Ret(SHARPEN_THIS_CALL *)(_Class *, _Args...);

    template<typename _Ret, typename... _Args>
    using UncheckedTruvialFunctionPtr = _Ret(SHARPEN_THIS_CALL *)(void *, _Args...);

    template<typename _Class, typename _Ret, typename... _Args>
    union InternalMemberFunctionConvertor
    {
        _Ret (_Class::*member_)(_Args...);
        sharpen::TrivialFunctionPtr<_Class, _Ret, _Args...> normal_;
    };

    template<typename _Class, typename _Ret, typename... _Args>
    union InternalConstMemberFunctionConvertor
    {
        _Ret (_Class::*member_)(_Args...) const;
        sharpen::TrivialFunctionPtr<const _Class, _Ret, _Args...> normal_;
    };

    template<typename _Class, typename _Ret, typename... _Args>
    inline auto InternalConvertToFunctionPtr(_Ret (_Class::*fp)(_Args...) const, int)
        -> sharpen::TrivialFunctionPtr<const _Class, _Ret, _Args...>
    {
        sharpen::InternalConstMemberFunctionConvertor<_Class, _Ret, _Args...> u;
        u.member_ = fp;
        return u.normal_;
    }

    template<typename _Class, typename _Ret, typename... _Args>
    inline auto InternalConvertToFunctionPtr(_Ret (_Class::*fp)(_Args...), ...)
        -> sharpen::TrivialFunctionPtr<_Class, _Ret, _Args...>
    {
        sharpen::InternalMemberFunctionConvertor<_Class, _Ret, _Args...> u;
        u.member_ = fp;
        return u.normal_;
    }

    template<typename _Fn>
    inline auto TrivialFunction(_Fn fn) -> decltype(sharpen::InternalConvertToFunctionPtr(fn, 0))
    {
        return sharpen::InternalConvertToFunctionPtr(fn, 0);
    }

    template<typename _Class, typename _Ret, typename... _Args>
    inline sharpen::UncheckedTruvialFunctionPtr<_Ret, _Args...>
    UncheckedTrivialFunctionCast(sharpen::TrivialFunctionPtr<_Class, _Ret, _Args...> fp)
    {
        return reinterpret_cast<sharpen::UncheckedTruvialFunctionPtr<_Ret, _Args...>>(fp);
    }

    template<typename _Fn>
    inline auto UncheckedTrivialFunction(_Fn fp)
        -> decltype(sharpen::UncheckedTrivialFunctionCast(sharpen::TrivialFunction(fp)))
    {
        return sharpen::UncheckedTrivialFunctionCast(sharpen::TrivialFunction(fp));
    }

    template<typename _Ret, typename... _Args>
    class MethodInvoker
    {
    private:
        using Self = sharpen::MethodInvoker<_Ret, _Args...>;

        sharpen::UncheckedTruvialFunctionPtr<_Ret, _Args...> method_;

    public:
        template<
            typename _Fn,
            typename _Check =
                decltype(std::declval<sharpen::UncheckedTruvialFunctionPtr<_Ret, _Args...> &>() =
                             sharpen::UncheckedTrivialFunction(std::declval<_Fn &>()))>
        explicit MethodInvoker(_Fn fn)
            : method_(sharpen::UncheckedTrivialFunction(fn))
        {
        }

        MethodInvoker(const Self &other) = default;

        MethodInvoker(Self &&other) noexcept
            : method_(other.method_)
        {
            other.method_ = nullptr;
        }

        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp, *this);
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if (this != std::addressof(other))
            {
                this->method_ = other.method_;
                other.method_ = nullptr;
            }
            return *this;
        }

        ~MethodInvoker() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline _Ret Invoke(void *thiz, _Args... args) const
        {
            assert(this->method_ != nullptr);
            return this->method_(thiz, args...);
        }

        inline _Ret InvokeConst(const void *thiz, _Args... args) const
        {
            assert(this->method_ != nullptr);
            return this->method_(const_cast<void *>(thiz), args...);
        }
    };
}   // namespace sharpen

#undef SHARPEN_THIS_CALL
#endif