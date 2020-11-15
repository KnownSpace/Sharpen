#pragma once
#ifndef _SHARPEN_PIPE_HPP
#define _SHARPEN_PIPE_HPP

#include "SystemMacro.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "FileTypeDef.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    class Pipe:public sharpen::Noncopyable
    {
    private:
       
        sharpen::FileHandle handle_;
    public:
    };
}

#endif
