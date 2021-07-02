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

        sharpen::ErrorCode cancelErr_;
    protected:
        virtual void DoExecute(sharpen::FileHandle handle,bool &executed,bool &blocking) override;

        void CancelCallback() noexcept;
    public:
        explicit PosixIoWriter(sharpen::ErrorCode cancelErr);

        ~PosixIoWriter() noexcept;
    };
}

#endif

#endif