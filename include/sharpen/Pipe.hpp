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
        using Self = sharpen::Pipe;
       
        sharpen::FileHandle handle_;
    public:

        Pipe();

        Pipe(Self &&other) noexcept;

        ~Pipe();

        void Read();

        void Write();

        Self &operator=(Self &&other) noexcept;
    };
}

#endif
