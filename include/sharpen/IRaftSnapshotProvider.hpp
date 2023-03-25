#pragma once
#ifndef _SHARPEN_IRAFTSNAPSHOTPROVIDER_HPP
#define _SHARPEN_IRAFTSNAPSHOTPROVIDER_HPP

#include <memory>

#include "IRaftSnapshot.hpp"

namespace sharpen
{
    class IRaftSnapshotProvider
    {
    private:
        using Self = sharpen::IRaftSnapshotProvider;
    protected:
    public:
    
        IRaftSnapshotProvider() noexcept = default;
    
        IRaftSnapshotProvider(const Self &other) noexcept = default;
    
        IRaftSnapshotProvider(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRaftSnapshotProvider() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual std::unique_ptr<sharpen::IRaftSnapshot> GetSnapshot() const = 0;
    };   
}

#endif