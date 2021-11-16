#pragma once
#ifndef _SHARPEN_FORCEINLINE_HPP
#define _SHARPEN_FORCEINLINE_HPP

#include "CompilerInfo.hpp"

//msvc
#ifdef SHARPEN_COMPILER_MSVC
#define SHARPEN_FORCEINLINE __forceinline
//GCC & clang
#elif defined(SHARPEN_COMPILER_GCC) || defined(SHARPEN_COMPILER_CLANG)
#define SHARPEN_FORCEINLINE inline __attribute__((__always_inline__))
#else
#define SHARPEN_FORCEINLINE inline
#endif

#endif