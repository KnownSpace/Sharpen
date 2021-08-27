#pragma once
#ifndef _SHARPEN_CONSOLE_HPP
#define _SHARPEN_CONSOLE_HPP

#include <cstdio>
#include <string>

#include "Converter.hpp"
#include "CompilerInfo.hpp"

namespace sharpen
{
    template<typename T>
    struct SpecialPrinter
    {};
    
    template<typename _T,typename _IsNum = typename std::enable_if<std::is_integral<_T>::value>::type>
    struct HexFormat:public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::HexFormat<_T>;

        const _T &val_;
    public:
        explicit HexFormat(const _T &val) noexcept
            :val_(val)
        {}

        HexFormat(const Self &other) noexcept
            :val_(other.val_)
        {}

        ~HexFormat() noexcept = default;

        const _T &Value() const noexcept
        {
            return this->val_;
        }
    };
    
    template<typename _T,typename _IsNum = typename std::enable_if<std::is_integral<_T>::value>::type>
    struct DecFormat:public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::DecFormat<_T>;

        const _T &val_;
    public:
        explicit DecFormat(const _T &val) noexcept
            :val_(val)
        {}

        DecFormat(const Self &other) noexcept
            :val_(other.val_)
        {}

        ~DecFormat() noexcept = default;

        const _T &Value() const noexcept
        {
            return this->val_;
        }
    };

    template<typename _T,typename _IsNum = typename std::enable_if<std::is_integral<_T>::value>::type>
    struct BinFormat:public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::BinFormat<_T>;

        const _T &val_;
    public:
        explicit BinFormat(const _T &val) noexcept
            :val_(val)
        {}

        BinFormat(const Self &other) noexcept
            :val_(other.val_)
        {}

        ~BinFormat() noexcept = default;

        const _T &Value() const noexcept
        {
            return this->val_;
        }
    };

    template<typename _T>
    struct SpecialPrinter<sharpen::HexFormat<_T>>
    {
        static void Print(FILE *file,const sharpen::HexFormat<_T> &val)
        {
            char buf[18] = {0};
            sharpen::Itoa(val.Value(),16,buf);
            std::fputs(buf,file);
        }
    };

    template<typename _T>
    struct SpecialPrinter<sharpen::DecFormat<_T>>
    {
        static void Print(FILE *file,const sharpen::DecFormat<_T> &val)
        {
            char buf[24] = {0};
            sharpen::Itoa(val.Value(),8,buf);
            std::fputs(buf,file);
        }
    };

    template<typename _T>
    struct SpecialPrinter<sharpen::BinFormat<_T>>
    {
        static void Print(FILE *file,const sharpen::BinFormat<_T> &val)
        {
            char buf[66] = {0};
            sharpen::Itoa(val.Value(),2,buf);
            std::fputs(buf,file);
        }
    };

    struct TypePrinter
    {
    private:
        static void IsStr(std::string);

        static void IsCstr(const char *);

        static void IsBool(const bool);
    public:
        //T is string
        template<typename _T,typename _IsStr = decltype(TypePrinter::IsStr(std::declval<_T>())),typename _RawType = typename std::remove_reference<_T>::type,typename _NonCstr = typename std::enable_if<!std::is_pointer<_RawType>::value && !std::is_array<_RawType>::value>::type>
        static void Print(FILE *file,_T &&str,...)
        {
            std::fputs(str.c_str(),file);
        }

        //T is int or uint
        template<typename _T,typename _IsNum = typename std::enable_if<std::is_integral<typename std::remove_reference<_T>::type>::value>::type>
        static void Print(FILE *file,_T &&val,int,...)
        {
            char buf[22] = {0};
            sharpen::Itoa(val,10,buf);
            std::fputs(buf,file);
        }

        //T is float or double
        template<typename _T,typename _IsFloat = typename std::enable_if<std::is_floating_point<typename std::remove_reference<_T>::type>::value>::type>
        static void Print(FILE *file,_T &&val,int,int,...)
        {
            std::fprintf(file,"%f",val);
        }

        //T is pointer
        template<typename _T,typename _IsPtr = typename std::enable_if<std::is_pointer<_T>::value>::type>
        static void Print(FILE *file,_T &&ptr,int,int,int,...)
        {
            std::fprintf(file,"%p",ptr);
        }

        //T is c-style string
        template<typename _T,typename _IsCstr = decltype(sharpen::TypePrinter::IsCstr(std::declval<_T>()))>
        static void Print(FILE *file,_T &&str,int,int,int,int,...)
        {
            const char *cstr = str;
            if (cstr)
            {
                std::fputs(cstr,file);
                return;
            }
            std::fprintf(file,"%p",cstr);
        }

        //T is bool
        template<typename _T,typename _IsBool = typename std::enable_if<std::is_same<bool,typename std::remove_const<typename std::remove_reference<_T>::type>::type>::value>::type>
        static void Print(FILE *file,_T &&val,int,int,int,int,int,...)
        {
            const char *str = "true";
            if (!val)
            {
                str = "false";
            }
            std::fputs(str,file);
        }

        //SpecialPrinter
        template<typename _T,typename _RawType = typename std::remove_reference<_T>::type,typename _HasSpecialPrinter = decltype(&sharpen::SpecialPrinter<_RawType>::Print)>
        static void Print(FILE *file,_T &&val,int,int,int,int,int,int,...)
        {
            sharpen::SpecialPrinter<_RawType>::Print(file,std::forward<_T>(val));
        }
    };
    

    template<typename _T,typename ..._Ts>
    struct ConsolePrinter
    {
        static void Print(_T &&arg,_Ts &&...args)
        {
            sharpen::TypePrinter::Print(stdout,std::forward<_T>(arg),0,0,0,0,0,0);
            sharpen::ConsolePrinter<_Ts...>::Print(std::forward<_Ts>(args)...);
        }

        static void Perror(_T &&arg,_Ts &&...args)
        {
            sharpen::TypePrinter::Print(stderr,std::forward<_T>(arg),0,0,0,0,0,0);
            sharpen::ConsolePrinter<_Ts...>::Print(std::forward<_Ts>(args)...);
        }
    };

    template<typename _T>
    struct ConsolePrinter<_T>
    {
        static void Print(_T &&arg)
        {
            sharpen::TypePrinter::Print(stdout,std::forward<_T>(arg),0,0,0,0,0,0);
        }

        static void Perror(_T &&arg)
        {
            sharpen::TypePrinter::Print(stderr,std::forward<_T>(arg),0,0,0,0,0,0);
        }
    };
    
    template<typename _T,typename _Check = decltype(sharpen::TypePrinter::Print(stdout,std::declval<_T>(),0,0,0,0,0,0))>
    static void PrintCheck(_T &&);
    
    template<typename ..._Ts>
    using PrintTest = void;

    template<typename _T,typename ..._Ts,typename _Check = PrintTest<decltype(sharpen::PrintCheck(std::declval<_T>())),decltype(sharpen::PrintCheck(std::declval<_Ts>()))...>>
    inline void Print(_T &&arg,_Ts &&...args)
    {
        sharpen::ConsolePrinter<_T,_Ts...>::Print(std::forward<_T>(arg),std::forward<_Ts>(args)...);
    }

    template<typename _T,typename ..._Ts,typename _Check = PrintTest<decltype(sharpen::PrintCheck(std::declval<_T>())),decltype(sharpen::PrintCheck(std::declval<_Ts>()))...>>
    inline void Perror(_T &&arg,_Ts &&...args)
    {
        sharpen::ConsolePrinter<_T,_Ts...>::Perror(std::forward<_T>(arg),std::forward<_Ts>(args)...);
    }
}

#endif