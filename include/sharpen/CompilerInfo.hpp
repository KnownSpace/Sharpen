#pragma once

#ifndef _SHARPEN_COMPILERINFO_HPP
#define _SHARPEN_COMPILERINFO_HPP
#ifdef _MSC_VER
#define SHARPEN_COMPILER_MSVC
#endif
#ifdef __GNUC__
#define SHARPEN_COMPILER_GCC
#endif
#ifdef __clang_
#define SHARPEN_COMPILER_CLANG
#endif
#endif