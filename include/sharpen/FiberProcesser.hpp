#pragma once
#ifndef _SHARPEN_PROCESSERPOOL_HPP
#define _SHARPEN_PROCESSERPOOL_HPP

#include <thread>
#include <cassert>

#include "FiberScheduler.hpp"
#include "AsyncOps.hpp"

namespace sharpen
{
    class FiberProcesser:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::FiberProcesser;

        std::thread thread_;

        sharpen::FiberScheduler *scheduler_;

        void ProcesserEntry();
    public:
        FiberProcesser();

        template<typename _Fn,typename ..._Args,typename = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        FiberProcesser(_Fn &&fn,_Args &&...args)
            :FiberProcesser()
        {
            std::function<void()> func = std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            sharpen::Launch([func]() mutable
            {
                try
                {
                    func();
                }
                catch(const std::exception& ignore)
                {
                    assert(ignore.what() == nullptr);
                    (void)ignore;
                }
                if (sharpen::FiberScheduler::IsProcesser())
                {
                    sharpen::FiberScheduler &scheduler = sharpen::FiberScheduler::GetScheduler();
                    scheduler.Stop();
                }
            });
        }

        FiberProcesser(Self &&other) noexcept;

        ~FiberProcesser() noexcept;

        Self &operator=(Self &&other) noexcept;

        void Stop() noexcept;

        void Join();

        void Detach();
    };
    
}

#endif