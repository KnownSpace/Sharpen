#pragma once
#ifndef _SHARPEN_FUNCTIONAL_HPP
#define _SHARPEN_FUNCTIONAL_HPP

#include <memory>
#include <functional>

namespace sharpen
{
    template<typename _Fn,typename _Obj,typename ..._Args,typename _R = decltype(std::declval<_Fn>()(std::declval<_Obj*>(),std::declval<_Args>()...))>
    inline _R BindSharedThis(_Fn &&fn,std::shared_ptr<_Obj> obj,_Args &&...args)
    {
        auto temp = std::bind(std::forward<_Fn>(fn),std::placeholders::_1,std::forward<_Args>(args)...);
        return [temp,obj]()
        {
            temp(obj.get());
        };
    }
}

#endif