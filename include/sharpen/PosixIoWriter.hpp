#pragma once
#ifndef _SHARPEN_POSIXWRITER_HPP
#define _SHARPEN_POSIXWRITER_HPP

#include "IPosixIoOperator.hpp"

#ifdef SHARPEN_IS_NIX

namespace sharpen
{
    class PosixIoWriter:public sharpen::IPosixIoOperator
    {
    private:
        using Mybase = sharpen::IPosixIoOperator;
    protected:
        virtual void NviExecute(sharpen::FileHandle handle,bool &executed,bool &blocking) override;
    public:
        PosixIoWriter() = default;

        ~PosixIoWriter() noexcept = default;
    };
}

#endif

#endif