#pragma once
#ifndef _SHARPEN_MICRORPCFIELD_HPP
#define _SHARPEN_MICRORPCFIELD_HPP

#include <cassert>
#include <cstring>

#include "ByteBuffer.hpp"
#include "MicroRpcVariable.hpp"
#include "IntOps.hpp"
#include "IteratorOps.hpp"

namespace sharpen
{
    //end flag 1 bit
    //size space 3 bit
    //type 4 bit
    struct MicroRpcFieldHeader
    {
        unsigned char end_:1;
        unsigned char sizeSpace_:3;
        unsigned char type_:4;
    };

    /*
    any type
    0       1
    +++++++++++++++++++++++++
    | Header| Size  | Data  |
    +++++++++++++++++++++++++

    header record meta data
    include:
    end flag: when == 1,this field is last one
    size space: 0-6,8 no 7
    type enum
    */

    class MicroRpcField
    {
    private:
        using Self = sharpen::MicroRpcField;

        sharpen::ByteBuffer data_;

        void InitHeader(sharpen::MicroRpcVariableType type);

        char *ComputeDataBody() noexcept;

        const char *ComputeDataBody() const noexcept;

        static sharpen::Size ComputeSizeSpace(sharpen::Size size) noexcept;
    public:
        MicroRpcField() = default;

        template<typename _T>
        explicit MicroRpcField(const sharpen::MicroRpcVariable<_T> &variable)
            :data_(1)
        {
            //init header
            this->InitHeader(sharpen::MicroRpcVariableTypeTrait<_T>::TypeEnum_);
            //compute size space
            sharpen::Size size{variable.GetSize()};
            sharpen::Size sizeSpace{sharpen::MicroRpcField::ComputeSizeSpace(size)};
            this->Header().sizeSpace_ = static_cast<unsigned char>(sizeSpace);
            //compute total size and extend buffer
            if (sizeSpace == 7)
            {
                sizeSpace += 1;
            }
            sharpen::Size totalSize{sizeSpace + variable.ComputeSize() + 1};
            this->data_.ExtendTo(totalSize);
            //copy size
            std::memcpy(this->data_.Data() + 1,&size,sizeSpace);
            //copy data
            //just ignore size argument
            //never throw exception
            variable.CopyTo(this->data_.Data() + 1 + sizeSpace,totalSize);
        }

        template<typename _T,typename _Check = typename sharpen::MicroRpcVariableTypeTrait<_T>::Type>
        explicit MicroRpcField(const _T &val)
        {
            this->data_.Reserve(1 + sizeof(val));
            //init header
            this->InitHeader(sharpen::MicroRpcVariableTypeTrait<_T>::TypeEnum_);
            //copy data
            this->data_.ExtendTo(1 + sizeof(val));
            sharpen::InternalMicroRpcVariableCopyTo<sizeof(val)>(this->RawData().Data() + 1,reinterpret_cast<const char*>(&val));
        }

        explicit MicroRpcField(const sharpen::MicroRpcVariable<void> &)
            :data_(1)
        {
            //init header
            this->InitHeader(sharpen::MicroRpcVariableType::Void);
            //do nothing
        }

        MicroRpcField(sharpen::MicroRpcFieldHeader header,const char *data,sharpen::Size size)
            :data_(1 + size)
        {
            this->Header() = header;
            std::memcpy(this->data_.Data() + 1,data,size);
        }

        template<typename _Iterator,typename _T = typename std::decay<decltype(*std::declval<_Iterator>())>::type,typename _Check = typename sharpen::MicroRpcVariableTypeTrait<_T>::Type>
        MicroRpcField(_Iterator begin,_Iterator end)
            :data_(1)
        {
            //init header
            this->InitHeader(sharpen::MicroRpcVariableTypeTrait<_T>::TypeEnum_);
            //compute size space
            sharpen::Size size{sharpen::GetRangeSize(begin,end)};
            sharpen::Size sizeSpace{sharpen::MicroRpcField::ComputeSizeSpace(size)};
            this->Header().sizeSpace_ = static_cast<unsigned char>(sizeSpace);
            //compute total size and extend buffer
            constexpr sharpen::Size typeSize{sharpen::MicroRpcVariableTypeTrait<_T>::Size_};
            if (sizeSpace == 7)
            {
                sizeSpace += 1;
            }
            this->data_.ExtendTo(sizeSpace + size*typeSize + 1);
            //copy size
            std::memcpy(this->data_.Data() + 1,&size,sizeSpace);
            char *data = this->RawData().Data() + 1 + sizeSpace;
            //copy data
            while (begin != end)
            {
                sharpen::InternalMicroRpcVariableCopyTo<typeSize>(data,reinterpret_cast<const char*>(&*begin));
                ++begin;
                data += typeSize;
            }
        }

        MicroRpcField(const Self &other) = default;

        MicroRpcField(Self &&other) noexcept = default;

        Self &operator=(const Self &other) = default;

        Self &operator=(Self &&other) noexcept = default;

        ~MicroRpcField() noexcept = default;

        void CopyTo(bool last,char *buf,sharpen::Size size) const;

        sharpen::ByteBuffer &RawData() noexcept
        {
            return this->data_;
        }

        template<typename _T,sharpen::MicroRpcVariableType _TypeEnum = sharpen::MicroRpcVariableTypeTrait<_T>::TypeEnum_>
        _T *Data()
        {
            if (this->Header().type_ != static_cast<unsigned char>(_TypeEnum))
            {
                throw std::logic_error("bad cast");
            }
            return reinterpret_cast<_T*>(this->ComputeDataBody());
        }

        template<typename _T,sharpen::MicroRpcVariableType _TypeEnum = sharpen::MicroRpcVariableTypeTrait<_T>::TypeEnum_>
        const _T *Data() const noexcept
        {
            if (this->Header().type_ != static_cast<unsigned char>(_TypeEnum))
            {
                throw std::bad_cast();
            }
            return reinterpret_cast<_T*>(this->ComputeDataBody());
        }

        const sharpen::ByteBuffer &RawData() const noexcept
        {
            return this->data_;
        }

        sharpen::MicroRpcFieldHeader &Header() noexcept
        {
            return *reinterpret_cast<sharpen::MicroRpcFieldHeader*>(this->data_.Data());
        }

        const sharpen::MicroRpcFieldHeader &Header() const noexcept
        {
            return *reinterpret_cast<const sharpen::MicroRpcFieldHeader*>(this->data_.Data());
        }

        sharpen::Size GetRawSize() const noexcept
        {
            return this->data_.GetSize();
        }

        sharpen::Uint64 GetSize() const;
    };
}

#endif