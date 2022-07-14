#pragma once
#ifndef _SHARPEN_LRULIST_HPP
#define _SHARPEN_LRULIST_HPP

#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen
{
    template<typename _T>
    class FifoList
    {
    private:
        using Self = sharpen::FifoList;
    public:
        struct Node
        {
        private:

            _T obj_;
            Node *next_;
            Node *prev_;
        public:
            template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
            Node(Node *next,Node *prev,_Args &&...args)
                :obj_(std::forward<_Args>(args)...)
                ,next_(next)
                ,prev_(prev)
            {}

            Node(Node &&other) noexcept = default;

            ~Node() noexcept = default;

            inline Node &operator=(Node &&other) noexcept
            {
                if(this != std::addressof(other))
                {
                    this->obj_ = std::move(other.obj_);
                    this->next_ = other.next_;
                    this->prev_ = other.prev_;
                }
                return *this;
            }

            inline _T &Object() noexcept
            {
                return this->obj_;
            }  

            inline const _T &Object() const noexcept
            {
                return this->obj_;
            }

            inline Node *GetNext() const noexcept
            {
                return this->next_;
            }

            inline void SetNext(Node *next) noexcept
            {
                this->next_ = next;
            }

            inline Node *GetPrev() const noexcept
            {
                return this->prev_;
            }

            inline void SetPrev(Node *prev) noexcept
            {
                this->prev_ = prev;
            }
        };
    private:

        Node *head_;
        Node *tail_;
    public:
    
        FifoList()
            :head_(nullptr)
            ,tail_(nullptr)
        {}
    
        FifoList(Self &&other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept;
    
        ~FifoList() noexcept
        {
            Node *head{this->head_};
            while(head)
            {
                Node *next{head->GetNext()};
                delete head;
                head = next;
            }
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        inline Node *Emplace(_Args &&...args)
        {
            Node *node{new Node{nullptr,this->tail_,std::forward<_Args>(args)...}};
            if(!this->head_)
            {
                this->head_ = node;
                this->tail_ = node;
            }
            else
            {
                this->tail_->SetNext(node);
                this->tail_ = node;
            }
        }

        inline void Move(Node *node) noexcept
        {
            if(this->tail_ != node)
            {
                this->tail_->SetNext(node);
                Node *prev{node->GetPrev()};
                prev->SetNext(node->GetNext());
                Node *next{node->GetNext()};
                next->SetPrev(prev);
                node->SetNext(nullptr);
                this->tail_->SetNext(node);
            }
        }

        inline void Pop() noexcept
        {
            if(this->head_ == this->tail_)
            {
                Node *node{this->head_};
                this->head_ = nullptr;
                this->tail_ = nullptr;
                if(node)
                {
                    delete node;
                }
                return;
            }
            Node *head{this->head_};
            this->head_ = head->GetNext();
            delete head;
        }

        inline Node *GetHead() const noexcept
        {
            return this->head_;
        }

        inline Node *GetTail() const noexcept
        {
            return this->tail_;
        }
    };
}

#endif