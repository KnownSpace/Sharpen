#pragma once
#ifndef _SIMPLETEST_TESTRESULT_HPP
#define _SIMPLETEST_TESTRESULT_HPP

#include <utility>
#include <string>

namespace simpletest
{
    class TestResult
    {
    private:
        using Self = simpletest::TestResult;
    
        bool status_;
        std::string reason_;
    public:
    
        TestResult() noexcept
            :status_(true)
            ,reason_()
        {}

        explicit TestResult(std::string reason) noexcept
            :status_(false)
            ,reason_(std::move(reason))
        {}
    
        TestResult(const Self &other) = default;
    
        TestResult(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->status_ = other.status_;
                this->reason_ = std::move(other.reason_);
            }
            return *this;
        }
    
        ~TestResult() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline bool Success() const noexcept
        {
            return this->status_;
        }

        inline bool Fail() const noexcept
        {
            return !this->status_;
        }

        inline std::string &Reason() noexcept
        {
            return this->reason_;
        }
        
        inline const std::string &Reason() const noexcept
        {
            return this->reason_;
        }
    };
}

#endif