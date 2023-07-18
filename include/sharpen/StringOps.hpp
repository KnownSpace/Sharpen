#pragma once
#ifndef _SHARPEN_STRINGOPS_HPP
#define _SHARPEN_STRINGOPS_HPP

#include <string>

namespace sharpen {
    struct Strings {
        static bool BeginWith(const std::string &str,const std::string &prefix) noexcept;

        static bool EndWith(const std::string &str,const std::string &suffix) noexcept;
    };
}

#endif