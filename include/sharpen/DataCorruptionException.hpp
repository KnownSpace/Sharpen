#pragma once
#ifndef _SHARPEN_DATACORRUPTIONEXCEPTION_HPP
#define _SHARPEN_DATACORRUPTIONEXCEPTION_HPP

#include <exception>

namespace sharpen
{
    template<typename _Exception,bool _IsEmpty>
    class InternalDataCorruptionException:public _Exception
    {
    private:
    
        using Self = InternalDataCorruptionException<_Exception,_IsEmpty>;
    
        const char *msg_;
    public:
        InternalDataCorruptionException() noexcept = default;
    
        explicit InternalDataCorruptionException(const char *msg)
            :msg_(msg)
        {}
    
        InternalDataCorruptionException(const Self &other) noexcept = default;
    
        InternalDataCorruptionException(Self &&other) noexcept = default;
    
        ~InternalDataCorruptionException() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual const char *what() const noexcept override
        {
            return this->msg_;
        }
    };
    
    //msvc exception
    template<typename _Exception>
    class InternalDataCorruptionException<_Exception,false>:public _Exception
    {
    private:
    
        using Self = InternalDataCorruptionException<_Exception,false>;
        using Base = _Exception;
    public:
        InternalDataCorruptionException() noexcept = default;
    
        explicit InternalDataCorruptionException(const char *msg) noexcept
            :Base(msg)
        {}
    
        InternalDataCorruptionException(const Self &other) noexcept = default;
    
        InternalDataCorruptionException(Self &&other) noexcept = default;
    
        ~InternalDataCorruptionException() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };
    
    //if sizeof(std::exception) == sizeof(void*),std::exception is a interface class
    //exception class defination
    class DataCorruptionException:public InternalDataCorruptionException<std::exception,sizeof(std::exception) == sizeof(void*)>
    {
    private:
        using Self = DataCorruptionException;
        using Base = InternalDataCorruptionException<std::exception,sizeof(std::exception) == sizeof(void*)>;
    public:
        DataCorruptionException() noexcept = default;
    
        explicit DataCorruptionException(const char *msg) noexcept
            :Base(msg)
        {}
    
        DataCorruptionException(const Self &other) noexcept = default;
    
        DataCorruptionException(Self &&other) noexcept = default;
    
        ~DataCorruptionException() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };
}

#endif