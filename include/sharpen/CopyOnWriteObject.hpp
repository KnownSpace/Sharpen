#pragma once
#ifndef _SHARPEN_COPYONWRITEOBJECT_HPP
#define _SHARPEN_COPYONWRITEOBJECT_HPP

#include <utility>
#include <memory>
#include <mutex>
#include <functional>
#include <cassert>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"

namespace sharpen
{

    template<typename _T,typename _Lock = sharpen::SpinLock>
    class CopyOnWriteObject:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Lock = _Lock;
        using Ptr = std::shared_ptr<_T>;
        using Writer = std::function<void(Ptr&)>;

        Ptr obj_;
        Lock lock_;
    public:

        template<typename ..._Args>
        CopyOnWriteObject(_Args &&...args)
            :obj_(std::make_shared<_T>(std::forward<_Args>(args)...))
            ,lock_()
        {}

        ~CopyOnWriteObject() = default;

        Ptr Read() const
        {
            Ptr p;
            {
                std::unique_lock<Lock> lock(this->lock_);
                p = this->obj_;
                assert(!p.unique());
            }
            return p;
        }

        void Write(Writer writer)
        {
            {
                std::unique_lock<Lock> lock(this->lock_);
                if (!this->obj_.unique())
                {
                    Ptr p = std::make_shared<_T>(*this->obj_);
                    this->obj_ = std::move(p);
                }
                assert(this->obj_.unique());
                writer(this->obj_);
            }
        }
    };
}
#endif