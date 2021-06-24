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

        sharpen::ErrorCode cancelErr_;
    protected:
        virtual void DoExecute(sharpen::FileHandle handle,bool &blocking) override;

        void CancelCallback() noexcept;
    public:
        explicit PosixIoReader(sharpen::ErrorCode cancelErr);

        ~PosixIoReader() noexcept;
    };
    
}

#endif

#endif