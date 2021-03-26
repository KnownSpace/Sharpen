#pragma once
#ifndef _SHARPEN_RESUMER_HPP
#define _SHARPEN_RESUMER_HPP

#include <csetjmp>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "ForceInline.hpp"

namespace sharpen
{
    class Resumer:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        std::jmp_buf buf_;
    public:
        enum class ReturnReason 
        {
            Recorded,
            Resume
        };

        Resumer() = default;

        ~Resumer() noexcept = default;

        void Resume()
        {
            std::longjmp(this->buf_,1);
        }

        SHARPEN_FORCEINLINE ReturnReason Record()
        {
            if (std::setjmp(this->buf_) == 0)
            {
                return ReturnReason::Recorded;
            }
            return ReturnReason::Resume;
        }
    };
    
}

#endif