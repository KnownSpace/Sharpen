#pragma once
#ifndef _SHARPEN_FIBERLOCALSTORAGE_HPP
#define _SHARPEN_FIBERLOCALSTORAGE_HPP

#include "Noncopyable.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>

namespace sharpen
{
    class FiberLocalStorage : public sharpen::Noncopyable
    {
    private:
        using Dtor = void (*)(void *);

        struct AnyPointer
        {
            void *pointer_;
            Dtor dtor_;
        };

        using Self = sharpen::FiberLocalStorage;
        using Map = std::map<std::size_t, AnyPointer>;

        static std::atomic_size_t nextIndex_;

        std::unique_ptr<Map> storage_;

    public:
        FiberLocalStorage() noexcept;

        FiberLocalStorage(Self &&other) noexcept = default;

        Self &operator=(Self &&other) noexcept;

        ~FiberLocalStorage() noexcept;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void *Lookup(std::size_t index) const noexcept;

        void Put(std::size_t index, void *p, Dtor dtor);

        void Remove(std::size_t index) noexcept;

        void Erase(std::size_t index) noexcept;

        static std::size_t GetNextIndex() noexcept;
    };
}   // namespace sharpen

#endif