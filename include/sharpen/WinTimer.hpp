#pragma once
#ifndef _SHARPEN_WINTIMER_HPP
#define _SHARPEN_WINTIMER_HPP

#include "SystemMacro.hpp"
#ifdef SHARPEN_IS_WIN

#include <atomic>

#include "ITimer.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#define SHARPEN_HAS_WAITABLETIMER
namespace sharpen
{
     class WinTimer:public sharpen::ITimer,public sharpen::Noncopyable,public sharpen::Nonmovable
     {
     private:
          using Myhandle = void *;
          using Mybase = sharpen::ITimer;

          struct WaitStruct
          {
               sharpen::WinTimer *timer_;
               sharpen::Size localTick_;
          };
          

          Myhandle handle_;
          sharpen::Future<void> *future_;
          std::atomic_size_t tick_;

          static void CompleteFuture(void *arg, DWORD, DWORD);
     public:
          WinTimer();

          virtual ~WinTimer() noexcept;

          virtual void WaitAsync(sharpen::Future<void> &future, sharpen::Uint64 waitMs) override;

          virtual void Cancel() override;
     };
}

#endif
#endif