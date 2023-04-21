#pragma once
#ifndef _SHARPEN_NOEXCEPTINVOKE_HPP
#define _SHARPEN_NOEXCEPTINVOKE_HPP

#include <functional>

namespace sharpen
{
    // crash if an exception was throw
    template<typename _Fn, typename... _Args>
    inline auto NonexceptInvoke(_Fn &&fn, _Args &&...args) noexcept
        -> decltype(fn(std::forward<_Args>(args)...))
    {
        return fn(std::forward<_Args>(args)...);
    }
}   // namespace sharpen

#endif