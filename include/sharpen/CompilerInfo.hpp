#pragma once

#ifndef _SHARPEN_COMPILERINFO_HPP
#define _SHARPEN_COMPILERINFO_HPP
#ifdef _MSC_VER
#define SHARPEN_COMPILER_MSVC
#elif (defined __GNUC__)
#define SHARPEN_COMPILER_GCC
#elif (defined __clang__)
#define SHARPEN_COMPILER_CLANG
#endif
#endif