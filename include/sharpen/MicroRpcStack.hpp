#pragma once
#ifndef _SHARPEN_MICRORPCSTACK_HPP
#define _SHARPEN_MICRORPCSTACK_HPP

#include <forward_list>

#include "MicroRpcField.hpp"

namespace sharpen
{
    class MicroRpcStack
    {
    private:
        using Self = sharpen::MicroRpcStack;
        using Field = sharpen::MicroRpcField;
        using FieldsType = std::forward_list<Field>;

        FieldsType fields_;

        void UnsafeCopyTo(char *data) const noexcept;
    public:
        MicroRpcStack() = default;

        MicroRpcStack(const Self &other) = default;

        MicroRpcStack(Self &&other) noexcept = default;

        Self &operator=(const Self &other) = default;

        Self &operator=(Self &&other) noexcept = default;

        ~MicroRpcStack() noexcept = default;

        FieldsType &Fields() noexcept
        {
            return this->fields_;
        }

        const FieldsType &Fields() const noexcept
        {
            return this->fields_;
        }

        sharpen::Size ComputeSize() const noexcept;

        void CopyTo(char *data,sharpen::Size size) const;

        inline void CopyTo(sharpen::ByteBuffer &buf)
        {
            this->CopyTo(buf,0);
        }

        void CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset);

        template<typename ..._Args,typename _Check = decltype(sharpen::MicroRpcField{std::declval<_Args>()...})>
        void Push(_Args &&...args)
        {
            this->fields_.emplace_front(std::forward<_Args>(args)...);
        }

        inline void Pop()
        {
            this->fields_.pop_front();
        }
    };
}

#endif