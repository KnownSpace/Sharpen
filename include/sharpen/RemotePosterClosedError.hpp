#pragma once
#ifndef _SHARPEN_ACTORCLOSEDERROR_HPP
#define _SHARPEN_ACTORCLOSEDERROR_HPP

#include <exception>

namespace sharpen
{

    template<typename _Exception,bool _IsEmpty>
    class InternalRemotePosterClosedError:public _Exception
    {
    private:
    
        using Self = InternalRemotePosterClosedError<_Exception,_IsEmpty>;
    
        const char *msg_;
    public:
        InternalRemotePosterClosedError() noexcept = default;
    
        explicit InternalRemotePosterClosedError(const char *msg)
            :msg_(msg)
        {}
    
        InternalRemotePosterClosedError(const Self &other) noexcept = default;
    
        InternalRemotePosterClosedError(Self &&other) noexcept = default;
    
        ~InternalRemotePosterClosedError() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual const char *what() const noexcept override
        {
            return this->msg_;
        }
    };
    
    //msvc exception
    template<typename _Exception>
    class InternalRemotePosterClosedError<_Exception,false>:public _Exception
    {
    private:
    
        using Self = InternalRemotePosterClosedError<_Exception,false>;
        using Base = _Exception;
    public:
        InternalRemotePosterClosedError() noexcept = default;
    
        explicit InternalRemotePosterClosedError(const char *msg) noexcept
            :Base(msg)
        {}
    
        InternalRemotePosterClosedError(const Self &other) noexcept = default;
    
        InternalRemotePosterClosedError(Self &&other) noexcept = default;
    
        ~InternalRemotePosterClosedError() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };
    
    //if sizeof(std::exception) == sizeof(void*),std::exception is a interface class
    //exception class defination
    class RemotePosterClosedError:public InternalRemotePosterClosedError<std::exception,sizeof(std::exception) == sizeof(void*)>
    {
    private:
        using Self = RemotePosterClosedError;
        using Base = InternalRemotePosterClosedError<std::exception,sizeof(std::exception) == sizeof(void*)>;
    public:
        RemotePosterClosedError() noexcept = default;
    
        explicit RemotePosterClosedError(const char *msg) noexcept
            :Base(msg)
        {}
    
        RemotePosterClosedError(const Self &other) noexcept = default;
    
        RemotePosterClosedError(Self &&other) noexcept = default;
    
        ~RemotePosterClosedError() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };   
}

#endif