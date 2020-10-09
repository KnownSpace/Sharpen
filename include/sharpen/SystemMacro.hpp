#pragma once
#ifndef _SHARPEN_SYSTEMMACRO_HPP
#define _SHARPEN_SYSTEMMACRO_HPP

//Windows
#if (defined (_Win32)) || (defined (_Win64))
#deinfe SHARPEN_IS_WIN
#endif

//Linux
#if (defined (_linux)) || (defined (_linux_))
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
