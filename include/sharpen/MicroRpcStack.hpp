#pragma once
#ifndef _SHARPEN_MICRORPCSTACK_HPP
#define _SHARPEN_MICRORPCSTACK_HPP

#include <forward_list>

#include "MicroRpcField.hpp"

namespace sharpen
{
    /*
    argument should be pushed from right to left
    proc name should be pushed in last
    stack view:
    +++++++++++++++++++++
    |ProcName|TypeName  |
    +++++++++++++++++++++
    |Arg1    |Type1     |
    +++++++++++++++++++++
    |...     |....      |
    +++++++++++++++++++++
    |ArgN    |TypeN     |
    +++++++++++++++++++++
    */
    class MicroRpcStack
    {
    private:
        using Self = sharpen::MicroRpcStack;
        using Field = sharpen::MicroRpcField;
        using FieldsType = std::forward_list<Field>;
        using Iterator = typename FieldsType::iterator;
        using ConstIterator = typename FieldsType::const_iterator;

        sharpen::Size size_;
        FieldsType fields_;

        void UnsafeCopyTo(char *data) const noexcept;
    public:
        MicroRpcStack();

        MicroRpcStack(const Self &other) = default;

        MicroRpcStack(Self &&other) noexcept;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        ~MicroRpcStack() noexcept = default;

        sharpen::Size ComputeSize() const noexcept;

        void CopyTo(char *data,sharpen::Size size) const;

        inline void CopyTo(sharpen::ByteBuffer &buf) const
        {
            this->CopyTo(buf,0);
        }

        void CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        template<typename ..._Args,typename _Check = decltype(sharpen::MicroRpcField{std::declval<_Args>()...})>
        void Push(_Args &&...args)
        {
            this->size_ += 1;
            this->fields_.emplace_front(std::forward<_Args>(args)...);
        }

        inline void Pop()
        {
            this->size_ -= 1;
            this->fields_.pop_front();
        }

        inline void Clear()
        {
            this->fields_.clear();
        }

        Iterator Begin()
        {
            return this->fields_.begin();
        }

        ConstIterator Begin() const
        {
            return this->fields_.cbegin();
        }

        Iterator End()
        {
            return this->fields_.end();
        }

        ConstIterator End() const
        {
            return this->fields_.cend();
        }

        sharpen::Size GetSize() const noexcept
        {
            return this->size_;
        }

        Field &Top()
        {
            return this->fields_.front();
        }

        const Field &Top() const
        {
            return this->fields_.front();
        }
    };
}

#endif