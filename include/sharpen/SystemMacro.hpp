#pragma once
#ifndef _SHARPEN_SYSTEMMACRO_HPP
#define _SHARPEN_SYSTEMMACRO_HPP

//Windows
#if (defined (_WIN32)) || (defined (_WIN64))
#define SHARPEN_IS_WIN
#define WIN32_LEAN_AND_MEAN
#endif

//Linux
#if (defined (_linux)) || (defined (_linux_)) || (defined (__linux__))
#define SHARPEN_IS_LINUX
#endif

//Unix
#if (defined (_unix)) || (defined (_unix_))
#define SHARPEN_IS_UNIX
#endif

//*nix
#if (defined (SHARPEN_IS_UNIX)) || (defined (SHARPEN_IS_LINUX))
#define SHARPEN_IS_NIX
#endif

#endif
