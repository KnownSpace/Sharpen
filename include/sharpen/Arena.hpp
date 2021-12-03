#pragma once
#ifndef _SHARPEN_ARENA_HPP
#define _SHARPEN_ARENA_HPP

#include <cstdlib>
#include <atomic>
#include <memory>
#include <cassert>
#include <limits>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "TypeDef.hpp"
#include "NoexceptIf.hpp"
#include "TypeTraits.hpp"
#include "SpinLock.hpp"

namespace sharpen
{
    class Arena:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        
        //max alloc
        static constexpr sharpen::Size maxAlloc_ = 16*1024;

        struct SmallBlock
        {
            SmallBlock* next_;
            char *end_;
            std::atomic<char*> curr_;
        };

        struct LargeBlock
        {
            LargeBlock* next_;
        };
        
        
        SmallBlock * volatile smallBlocks_;
        std::atomic<LargeBlock*> largeBlocks_;
        sharpen::SpinLock lock_;

        static SmallBlock *AllocSmallBlock(sharpen::Size size) noexcept;

        void *AllocSmall(sharpen::Size size) noexcept;

        void *AllocLarge(sharpen::Size size) noexcept;
    public:
        Arena()
            :Arena(maxAlloc_)
        {}

        explicit Arena(sharpen::Size blockSize);

        ~Arena() noexcept;

        inline void *Alloc(sharpen::Size size) noexcept
        {
            assert(size != 0);
            if (size > maxAlloc_)
            {
                return this->AllocLarge(size);
            }
            return this->AllocSmall(size);
        }

        template<typename _T,typename ..._Args>
        inline auto Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(new (nullptr) _T{std::declval<_Args>()...}) -> decltype(new (nullptr) _T{std::declval<_Args>()...})
        {
            _T *p = reinterpret_cast<_T*>(this->Alloc(sizeof(_T)));
            if(p == nullptr)
            {
                return nullptr;
            }
            return new (p) _T{std::forward<_Args>(args)...};
        }

        template<typename _T,typename ..._Args>
        inline auto ConstructArray(sharpen::Size count,_Args &&...args) SHARPEN_NOEXCEPT_IF(new (nullptr) _T{std::declval<_Args>()...}) -> decltype(new (nullptr) _T{std::declval<_Args>()...})
        {
            assert(count != 0);
            assert(std::numeric_limits<sharpen::Size>::max()/sizeof(_T) >= count);
            _T *p = reinterpret_cast<_T*>(this->Alloc(sizeof(_T) * count));
            if(p == nullptr)
            {
                return nullptr;
            }
            for (sharpen::Size i = 0; i < count; i++)
            {
                new (p + i) _T{std::forward<_Args>(args)...};
            }
            return p;
        }

        template<typename _T>
        inline static void Destruct(_T *obj) SHARPEN_NOEXCEPT_IF(std::declval<_T>().~_T())
        {
            obj->~_T();
        }
        
        template<typename _T>
        struct ObjectDeletor
        {
            void operator()(_T* ptr) const SHARPEN_NOEXCEPT_IF(sharpen::Arena::Destruct(ptr))
            {
                sharpen::Arena::Destruct(ptr);
            }
        };
        
        template<typename _T>
        struct ArrayDeletor
        {
        private:
            using Self = sharpen::Arena::ArrayDeletor<_T>;

            sharpen::Size count_;
        public:

            explicit ArrayDeletor(sharpen::Size count)
                :count_(count)
            {}

            ArrayDeletor(const Self &) noexcept = default;

            ArrayDeletor(Self &&) noexcept = default;

            Self &operator=(const Self &) noexcept = default;

            Self &operator=(Self &&) noexcept = default;
            
            ~ArrayDeletor() noexcept = default;

            void operator()(_T* ptr) const SHARPEN_NOEXCEPT_IF(sharpen::Arena::Destruct(ptr))
            {
                for (sharpen::Size i = 0; i < this->count_; i++)
                {
                    sharpen::Arena::Destruct(ptr + i);
                }
            }
        };

        template<typename _T,typename ..._Args>
        inline auto MakeUniqueObject(_Args &&...args) ->decltype(std::unique_ptr<_T,sharpen::Arena::ObjectDeletor<_T>>(new (nullptr) _T{std::forward<_Args>(args)...}))
        {
            auto *p = this->Construct<_T>(std::forward<_Args>(args)...);
            if(!p)
            {
                return nullptr;
            }
            return std::unique_ptr<_T,sharpen::Arena::ObjectDeletor<_T>>(p);
        }

        template<typename _T,typename ..._Args>
        inline auto MakeUniqueArray(sharpen::Size count,_Args &&...args) ->decltype(std::unique_ptr<_T,sharpen::Arena::ArrayDeletor<_T>>(new (nullptr) _T{std::forward<_Args>(args)...},sharpen::Arena::ArrayDeletor<_T>{0}))
        {
            auto *p = this->ConstructArray<_T>(count,std::forward<_Args>(args)...);
            if(!p)
            {
                count = 0;
            }
            return std::unique_ptr<_T,sharpen::Arena::ArrayDeletor<_T>>(p,sharpen::Arena::ArrayDeletor<_T>{count});
        }

        template<typename _T,typename ..._Args>
        inline auto MakeSharedObject(_Args &&...args) ->decltype(std::shared_ptr<_T>(new (nullptr) _T{std::forward<_Args>(args)...}))
        {
            auto *p = this->Construct<_T>(std::forward<_Args>(args)...);
            if(!p)
            {
                return nullptr;
            }
            return std::shared_ptr<_T>(p,sharpen::Arena::ObjectDeletor<_T>{});
        }

        template<typename _T,typename ..._Args>
        inline auto MakeSharedArray(sharpen::Size count,_Args &&...args) ->decltype(std::shared_ptr<_T>(new (nullptr) _T{std::forward<_Args>(args)...},sharpen::Arena::ArrayDeletor<_T>{0}))
        {
            auto *p = this->ConstructArray<_T>(count,std::forward<_Args>(args)...);
            if(!p)
            {
                count = 0;
            }
            return std::shared_ptr<_T>(p,sharpen::Arena::ArrayDeletor<_T>{count});
        }
    };
}

#endif