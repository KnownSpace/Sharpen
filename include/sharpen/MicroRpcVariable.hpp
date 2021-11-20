#pragma once
#ifndef _SHARPEN_MICRORPCVARIABLE_HPP
#define _SHARPEN_MICRORPCVARIABLE_HPP

#include <vector>
#include <stdexcept>

#include "TypeDef.hpp"
#include "ByteOrder.hpp"

namespace sharpen
{
    //4 bit
    enum class MicroRpcVariableType : unsigned char
    {
        Void = 0,
        Char = 1,
        Uchar = 2,
        Int16 = 3,
        Uint16 = 4,
        Int32 = 5,
        Uint32 = 6,
        Int64 = 7,
        Uint64 = 8,
        Double = 9,
        Float = 10
    };

    template <typename _T>
    struct MicroRpcVariableTypeTrait
    {};

    template <>
    struct MicroRpcVariableTypeTrait<void>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Void;
        static constexpr sharpen::Size Size_ = 0;
    };

    template <>
    struct MicroRpcVariableTypeTrait<char>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Char;
        static constexpr sharpen::Size Size_ = 1;
    };

    template <>
    struct MicroRpcVariableTypeTrait<unsigned char>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Uchar;
        static constexpr sharpen::Size Size_ = 1;
    };

    template <>
    struct MicroRpcVariableTypeTrait<sharpen::Int16>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Int16;
        static constexpr sharpen::Size Size_ = 2;
    };

    template <>
    struct MicroRpcVariableTypeTrait<sharpen::Uint16>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Uint16;
        static constexpr sharpen::Size Size_ = 2;
    };

    template <>
    struct MicroRpcVariableTypeTrait<sharpen::Uint32>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Uint32;
        static constexpr sharpen::Size Size_ = 4;
    };

    template <>
    struct MicroRpcVariableTypeTrait<sharpen::Uint64>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Uint64;
        static constexpr sharpen::Size Size_ = 8;
    };

    template <>
    struct MicroRpcVariableTypeTrait<sharpen::Int32>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Int32;
        static constexpr sharpen::Size Size_ = 4;
    };

    template <>
    struct MicroRpcVariableTypeTrait<sharpen::Int64>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Int64;
        static constexpr sharpen::Size Size_ = 8;
    };

    template<>
    struct MicroRpcVariableTypeTrait<double>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Double;
        static constexpr sharpen::Size Size_ = 8;
    };
    
    template<>
    struct MicroRpcVariableTypeTrait<float>
    {
        using Type = void;
        static constexpr sharpen::MicroRpcVariableType TypeEnum_ = sharpen::MicroRpcVariableType::Float;
        static constexpr sharpen::Size Size_ = 4;
    };

//[fallthrough]
#ifdef SHARPEN_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#elif (defined SHARPEN_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:26819)
#elif (defined SHARPEN_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#endif
    inline sharpen::Size GetMicroRpcTypeSize(sharpen::MicroRpcVariableType type) noexcept
    {
        switch (type)
        {
        case sharpen::MicroRpcVariableType::Void:
            return 0;
        case sharpen::MicroRpcVariableType::Char:
        case sharpen::MicroRpcVariableType::Uchar:
            return 1;
        case sharpen::MicroRpcVariableType::Int16:
        case sharpen::MicroRpcVariableType::Uint16:
            return 2;
        case sharpen::MicroRpcVariableType::Int32:
        case sharpen::MicroRpcVariableType::Uint32:
        case sharpen::MicroRpcVariableType::Float:
            return 4;
        case sharpen::MicroRpcVariableType::Int64:
        case sharpen::MicroRpcVariableType::Uint64:
        case sharpen::MicroRpcVariableType::Double:
            return 8;
        default:
            return 0;
        }
    }
#ifdef SHARPEN_COMPILER_GCC
#pragma GCC diagnostic push
#elif (defined SHARPEN_COMPILER_MSVC)
#pragma warning(pop)
#elif (defined SHARPEN_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif
    
    template<typename _T,typename _Check = void>
    class InternalMicroRpcVariable;

    template<sharpen::Size _Size>
    void InternalMicroRpcVariableCopyTo(char *dst,const char *src)
    {
        std::memcpy(dst,src,_Size);
#ifdef SHARPEN_IS_BIG_ENDIAN
        sharpen::ConvertEndian(dst,_Size);
#endif
    }

    template<>
    inline void InternalMicroRpcVariableCopyTo<1>(char *dst,const char *src)
    {
        *dst = *src;
    }

    template<>
    inline void InternalMicroRpcVariableCopyTo<0>(char *dst,const char *src)
    {
        (void)dst;
        (void)src;
    }

    template<typename _T>
    class InternalMicroRpcVariable<_T,typename sharpen::MicroRpcVariableTypeTrait<_T>::Type>
    {
    private:
        using Self = sharpen::InternalMicroRpcVariable<_T,typename sharpen::MicroRpcVariableTypeTrait<_T>::Type>;

        static constexpr sharpen::MicroRpcVariableType typeEnum_ = sharpen::MicroRpcVariableTypeTrait<_T>::TypeEnum_;

        static constexpr sharpen::Size typeSize_ = sharpen::MicroRpcVariableTypeTrait<_T>::Size_;

        std::vector<_T> data_;
    public:
        explicit InternalMicroRpcVariable(_T data)
            :data_()
        {
            this->data_.push_back(data);
        }

        template<typename _Iterator>
        InternalMicroRpcVariable(_Iterator begin,_Iterator end)
            :data_(begin,end)
        {}

        InternalMicroRpcVariable(const Self &other) = default;

        InternalMicroRpcVariable(Self &&other) = default;

        Self &operator=(const Self &other) = default;

        Self &operator=(Self &&other) noexcept = default;

        ~InternalMicroRpcVariable() noexcept = default;

        std::vector<_T> &RawData() noexcept
        {
            return this->data_;
        }

        const std::vector<_T> &RawData() const noexcept
        {
            return this->data_;
        }

        sharpen::Size GetSize() const noexcept
        {
            return this->data_.size();
        }

        sharpen::Size ComputeSize() const noexcept
        {
            return this->data_.size() * typeSize_;
        }

        void CopyTo(char *data,sharpen::Size size) const
        {
            if (size < this->ComputeSize())
            {
                throw std::logic_error("buffer too small");
            }
            const char *src = reinterpret_cast<const char*>(this->data_.data()); 
            for (size_t i = 0; i < size; i += typeSize_)
            {
                sharpen::InternalMicroRpcVariableCopyTo<typeSize_>(data + i,src + i);
            }
        }
    };

    template<>
    class InternalMicroRpcVariable<void,void>
    {
    private:

        using Self = sharpen::InternalMicroRpcVariable<void,sharpen::MicroRpcVariableType>;

        static constexpr sharpen::MicroRpcVariableType typeEnum_ = sharpen::MicroRpcVariableType::Void;
    public:
        /*nothing*/
    };

    template<typename _T>
    using MicroRpcVariable = sharpen::InternalMicroRpcVariable<_T>;
}

#endif