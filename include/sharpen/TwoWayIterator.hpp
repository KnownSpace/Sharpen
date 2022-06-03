#pragma once
#ifndef _SHARPEN_TWOWAYITERATOR_HPP
#define _SHARPEN_TWOWAYITERATOR_HPP

#include <iterator>
#include <cassert>
#include <type_traits>

#include "IteratorTemplate.hpp"

namespace sharpen
{
    struct TwoWayIteratorHelper
    {
    private:

        using Self = sharpen::TwoWayIteratorHelper;
    public:

        template<typename _Iterator>
        static auto InternalGetBegin(_Iterator &&iterator,int) -> decltype(iterator->Begin())
        {
            return iterator->Begin();
        }

        template<typename _Iterator>
        static auto InternalGetBegin(_Iterator &&iterator,...) -> decltype(iterator->begin())
        {
            return iterator->begin();
        }

        template<typename _Iterator>
        static auto InternalGetEnd(_Iterator &&iterator,int) -> decltype(iterator->End())
        {
            return iterator->End();
        }

        template<typename _Iterator>
        static auto InternalGetEnd(_Iterator &&iterator,...) -> decltype(iterator->end())
        {
            return iterator->end();
        }

        template<typename _Iterator>
        static auto GetBegin(_Iterator &&iterator) -> decltype(Self::InternalGetBegin(std::forward<_Iterator>(iterator),0))
        {
            return Self::InternalGetBegin(std::forward<_Iterator>(iterator),0);
        }

        template<typename _Iterator>
        static auto GetEnd(_Iterator &&iterator) -> decltype(Self::InternalGetEnd(std::forward<_Iterator>(iterator),0))
        {
            return Self::InternalGetEnd(std::forward<_Iterator>(iterator),0);
        }
    };
    

    template<typename _Iterator,typename _SubIterator = decltype(sharpen::TwoWayIteratorHelper::GetBegin(std::declval<_Iterator>())),typename _Value = typename std::remove_reference<decltype(*std::declval<_SubIterator>())>::type>
    class TwoWayIterator:public sharpen::DefaultIteratorTemplate<std::bidirectional_iterator_tag,_Value>
    {
    private:
        using Self = sharpen::TwoWayIterator<_Iterator,_SubIterator,_Value>;
    
        mutable _Iterator parent_;
        _Iterator parentEnd_;
        mutable _SubIterator sub_;
    public:
    
        TwoWayIterator(_Iterator parent,_Iterator parentEnd,_SubIterator sub)
            :parent_(parent)
            ,parentEnd_(parentEnd)
            ,sub_(sub)
        {}
    
        TwoWayIterator(const Self &other) = default;
    
        TwoWayIterator(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->parent_ = std::move(other.parent_);
                this->sub_ = std::move(other.sub_);
            }
            return *this;
        }
    
        ~TwoWayIterator() noexcept = default;

        _Value &operator*()
        {
            return *this->sub_;
        }

        const _Value &operator*() const
        {
            return *this->sub_;
        }

        _Value *operator->()
        {
            return &*this->sub_;
        }

        const _Value *operator->() const
        {
            return &*this->sub_;
        }

        Self &operator++()
        {
            assert(this->parent_ != this->parentEnd_);
            ++this->sub_;
            if(this->sub_ == sharpen::TwoWayIteratorHelper::GetEnd(this->parent_))
            {
                ++this->parent_;
                if(this->parent_ != this->parentEnd_)
                {
                    this->sub_ = sharpen::TwoWayIteratorHelper::GetBegin(this->parent_);
                }
            }
            return *this;
        }

        const Self &operator++() const
        {
            assert(this->parent_ != this->parentEnd_);
            ++this->sub_;
            if(this->sub_ == sharpen::TwoWayIteratorHelper::GetEnd(this->parent_))
            {
                ++this->parent_;
                if(this->parent_ != this->parentEnd_)
                {
                    this->sub_ = sharpen::TwoWayIteratorHelper::GetBegin(this->parent_);
                }
            }
            return *this;
        }

        Self operator++(int) const
        {
            Self tmp{*this};
            ++(*this);
            return tmp;
        }

        Self &operator--()
        {
            assert(this->parent_ != this->parentEnd_);
            if(this->sub_ == sharpen::TwoWayIteratorHelper::GetBegin(this->parent_))
            {
                --this->parent_;
                this->sub_ = --sharpen::TwoWayIteratorHelper::GetEnd(this->parent_);
            }
            else
            {
                --this->sub_;
            }
            return *this;
        }

        const Self &operator--() const
        {
            assert(this->parent_ != this->parentEnd_);
            if(this->sub_ == sharpen::TwoWayIteratorHelper::GetBegin(this->parent_))
            {
                --this->parent_;
                this->sub_ = --sharpen::TwoWayIteratorHelper::GetEnd(this->parent_);
            }
            else
            {
                --this->sub_;
            }
            return *this;
        }

        Self operator--(int) const
        {
            Self tmp{*this};
            --(*this);
            return tmp;
        }

        bool operator==(const Self &other) const
        {
            if(this->parent_ == other.parent_)
            {
                if(this->parent_ == this->parentEnd_)
                {
                    return true;
                }
                return this->sub_ == other.sub_;
            }
            return false;
        }

        bool operator!=(const Self &other) const
        {
            return !(*this == other);
        }
    };
}

#endif