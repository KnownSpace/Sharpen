#pragma once
#ifndef _SHARPEN_FIBERLOCAL_HPP
#define _SHARPEN_FIBERLOCAL_HPP

#include "Fiber.hpp"

namespace sharpen {
    template<typename _T>
    class FiberLocal {
    private:
        using Self = sharpen::FiberLocal<_T>;

        static void HeapDtor(void *p) noexcept {
            _T *obj{reinterpret_cast<_T *>(p)};
            delete obj;
        }

        std::size_t index_;

    public:
        FiberLocal() noexcept
            : index_(sharpen::FiberLocalStorage::GetNextIndex()) {
        }

        FiberLocal(const Self &other) noexcept = default;

        FiberLocal(Self &&other) noexcept = default;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                this->index_ = other.index_;
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept {
            if (this != std::addressof(other)) {
                this->index_ = other.index_;
            }
            return *this;
        }

        ~FiberLocal() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline _T *Lookup() const noexcept {
            sharpen::FiberLocalStorage &storage{sharpen::Fiber::GetLocalStorage()};
            return reinterpret_cast<_T *>(storage.Lookup(this->index_));
        }

        template<typename _Impl,
                 typename... _Args,
                 typename _Check = decltype(std::declval<_T *&>() = std::declval<_Impl *>(),
                                            _Impl{std::declval<_Args>()...})>
        inline _T *New(_Args &&...args) {
            sharpen::FiberLocalStorage &storage{sharpen::Fiber::GetLocalStorage()};
            _T *p{new (std::nothrow) _Impl{std::forward<_Args>(args)...}};
            if (!p) {
                throw std::bad_alloc{};
            }
            using FnPtr = void (*)(void *);
            FnPtr dtor{static_cast<FnPtr>(&Self::HeapDtor)};
            try {
                storage.Put(this->index_, p, dtor);
            } catch (const std::bad_alloc &rethrow) {
                delete p;
                (void)rethrow;
                throw;
            }
            return p;
        }

        template<typename... _Args, typename _Check = decltype(_T{std::declval<_Args>()...})>
        inline _T *New(_Args &&...args) {
            return this->New<_T, _Args...>(std::forward<_Args>(args)...);
        }

        inline _T *From(_T &obj) {
            sharpen::FiberLocalStorage &storage{sharpen::Fiber::GetLocalStorage()};
            storage.Put(&obj, nullptr);
            return &obj;
        }

        inline void Remove() noexcept {
            sharpen::FiberLocalStorage &storage{sharpen::Fiber::GetLocalStorage()};
            storage.Remove(this->index_);
        }

        inline void Erase() noexcept {
            sharpen::FiberLocalStorage &storage{sharpen::Fiber::GetLocalStorage()};
            storage.Erase(this->index_);
        }
    };
}   // namespace sharpen

#endif