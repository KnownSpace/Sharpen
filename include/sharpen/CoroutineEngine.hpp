#pragma once
#ifndef _SHARPEN_COROUTINEENGINE_HPP
#define _SHARPEN_COROUTINEENGINE_HPP

#include <memory>

#include "ExecuteContext.hpp"

namespace sharpen
{
  //one engine context per thread
  //this pointer is nullptr by default
  //if you use a coroutine function like Await()
  //we will make some initializational operations
  //it will call ConvertThreadToFiberEx and CreateFiberEx in windows and call getcontext and makecontext in *nix
  extern thread_local std::unique_ptr<sharpen::ExecuteContext> LocalEngineContext;
  
  //this is a internal class and you should never use it directly
  class CoroutineEngine;
}

#endif
