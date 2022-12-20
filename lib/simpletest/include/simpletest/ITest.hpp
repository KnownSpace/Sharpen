#pragma once
#ifndef _SIMPLETEST_ITEST_HPP
#define _SIMPLETEST_ITEST_HPP

#ifdef _MSC_VER
#define SIMPLETEST_MSVC
#elif (defined __GNUC__)
#define SIMPLETEST_GCC
#elif (defined __clang__)
#define SIMPLETEST_CLANG
#endif

#ifndef SIMPLETEST_MSVC
#include <cxxabi.h>

namespace simpletest
{
    constexpr inline static const char *Demangle(const char *name) noexcept
    {
        int status{0};
        //drop status
        (void)status;
        return abi::__cxa_demangle(name, 0, 0, &status)
    }   
}

#define SIMPLETEST_TYPENAME(x) simpletest::Demangle(typeid(x).name())
#else
#define SIMPLETEST_TYPENAME(x) typeid(x).name()+6
#endif

#include "TestResult.hpp"

namespace simpletest
{

    class ITest
    {
    private:
        using Self = simpletest::ITest;
    protected:

        inline static simpletest::TestResult Success() noexcept
        {
            return simpletest::TestResult{};
        }

        inline static simpletest::TestResult Fail(std::string reason) noexcept
        {
            return simpletest::TestResult{std::move(reason)};
        }

        inline static simpletest::TestResult Assert(bool expr,std::string reason) noexcept
        {
            if(expr)
            {
                return Success();
            }
            return Fail(std::move(reason));
        }

        std::string name_;
    public:
    
        ITest(std::string name) noexcept
            :name_(std::move(name))
        {}
    
        ITest(const Self &other) noexcept = delete;
    
        ITest(Self &&other) noexcept = delete;
    
        Self &operator=(const Self &other) noexcept = delete;
    
        Self &operator=(Self &&other) noexcept = delete;
    
        virtual ~ITest() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual simpletest::TestResult Run() noexcept = 0;
        
        inline const std::string &Name() const noexcept
        {
            return this->name_;
        }
    }; 

    template<typename _T>
    class ITypenamedTest:public simpletest::ITest
    {
    private:
        using Self = simpletest::ITypenamedTest<_T>;
        using Base = simpletest::ITest;
    protected:

    public:
    
        ITypenamedTest() noexcept
            :Base(SIMPLETEST_TYPENAME(_T))
        {}
    
        ITypenamedTest(const Self &other) noexcept = default;
    
        ITypenamedTest(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~ITypenamedTest() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };

    template<typename _T>
    constexpr inline static const char *GetReadableTypeName() noexcept
    {
        return SIMPLETEST_TYPENAME(_T);
    }  
}

#undef SIMPLETEST_TYPENAME
#endif