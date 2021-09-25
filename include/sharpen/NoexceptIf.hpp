#pragma once
#ifndef _SHARPEN_NOEXCEPTIF_HPP
#define _SHARPEN_NOEXCEPTIF_HPP

#define SHARPEN_NOEXCEPT_IF(expr) noexcept(noexcept(expr))

#endif