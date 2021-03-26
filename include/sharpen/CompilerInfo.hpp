#pragma once

#ifdef _MSC_VER
#define SHARPEN_COMPILER_MSVC
#elif defined(__GUNC__)
#define SHARPEN_COMPILER_GCC
#elif defined(__clang_)
#define SHARPEN_COMPILER_CLANG
#endif