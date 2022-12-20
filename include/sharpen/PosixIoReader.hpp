#pragma once
#ifndef _SHARPEN_POSIXREADER_HPP
#define _SHARPEN_POSIXREADER_HPP

#include "IPosixIoOperator.hpp"

#ifdef SHARPEN_IS_NIX

namespace sharpen
{
    class PosixIoReader:public sharpen::IPosixIoOperator
    {
    private:
        using Mybase = sharpen::IPosixIoOperator;
    protected:
        virtual void NviExecute(sharpen::FileHandle handle,bool &executed,bool &blocking) override;
    public:
        PosixIoReader() = default;

        ~PosixIoReader() noexcept = default;
    };
    
}

#endif

#endif