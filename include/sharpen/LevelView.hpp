#pragma once
#ifndef _SHARPEN_LEVELVIEW_HPP
#define _SHARPEN_LEVELVIEW_HPP

/*
+------------+
| Item count | varint
+------------+
| Item 1     |
+------------+
|    ...     |
+------------+
| Item N     |
+------------+
*/

#include "LevelViewItem.hpp"
#include "Optional.hpp"

namespace sharpen
{
    class LevelView
    {
    private:
        using Self = sharpen::LevelView;
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
        using Items = std::vector<sharpen::LevelViewItem>;
    public:
        using Iterator = typename Items::iterator;
        using ConstIterator = typename Items::const_iterator;
        using ReverseIterator = typename Items::reverse_iterator;
        using ConstReverseIterator = typename Items::const_reverse_iterator;
    private:
    
        sharpen::Uint64 id_;
        Items items_;
        Comparator comp_;

        static bool Comp(const sharpen::LevelViewItem &item,const sharpen::ByteBuffer &key) noexcept;

        static bool WarpComp(Comparator comp,const sharpen::LevelViewItem &item,const sharpen::ByteBuffer &key) noexcept;

        sharpen::Int32 CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;

        Iterator Find(const sharpen::ByteBuffer &key) noexcept;

        ConstIterator Find(const sharpen::ByteBuffer &key) const noexcept;
    public:

        explicit LevelView(sharpen::Uint64 id);
    
        LevelView(const Self &other) = default;
    
        LevelView(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->id_ = other.id_;
                this->items_ = std::move(other.items_);
                this->comp_ = other.comp_;
                other.id_ = 0;
                other.comp_ = nullptr;
            }
            return *this;
        }
    
        ~LevelView() noexcept = default;

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept;

        sharpen::Size ComputeSize() const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        sharpen::Optional<sharpen::Uint64> FindId(const sharpen::ByteBuffer &key) const;

        bool IsNotOverlapped(const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const noexcept;

        bool TryPut(sharpen::ByteBuffer beginKey,sharpen::ByteBuffer endKey,sharpen::Uint64 id);

        void Put(sharpen::ByteBuffer beginKey,sharpen::ByteBuffer endKey,sharpen::Uint64 id);

        void Delete(sharpen::Uint64 id);

        inline void Clear() noexcept
        {
            this->items_.clear();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->items_.size();
        }

        inline bool Empty() const noexcept
        {
            return this->items_.empty();
        }

        inline Iterator Begin() noexcept
        {
            return this->items_.begin();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->items_.begin();
        }

        inline Iterator End() noexcept
        {
            return this->items_.end();
        }

        inline ConstIterator End() const noexcept
        {
            return this->items_.end();
        }

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->items_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->items_.rbegin();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->items_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->items_.rend();
        }

        inline sharpen::Uint64 GetId() const noexcept
        {
            return this->id_;
        }

        inline void SetId(sharpen::Uint64 id) noexcept
        {
            this->id_ = id;
        }

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }

        inline void SetComparator(Comparator comp) noexcept
        {
            this->comp_ = comp;
        }
    };
}

#endif