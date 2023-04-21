#pragma once
#ifndef _SHARPEN_SIGNALMAP_HPP
#define _SHARPEN_SIGNALMAP_HPP

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include "FileTypeDef.hpp"
#include "SignalLock.hpp"

namespace sharpen
{
    class SignalMap
    {
    private:
        using Self = sharpen::SignalMap;
        using Map = std::map<std::int32_t, std::vector<sharpen::FileHandle>>;
        using ReMap = std::map<sharpen::FileHandle, std::vector<std::int32_t>>;
        using Lock = sharpen::SignalLock;

        Map map_;
        ReMap remap_;
        mutable sharpen::SignalLock lock_;

    public:
        SignalMap();

        ~SignalMap() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Register(sharpen::FileHandle handle, std::int32_t *sigs, std::size_t sigSize);

        inline void Register(sharpen::FileHandle handle, std::int32_t sig)
        {
            this->Register(handle, &sig, 1);
        }

        void Raise(std::int32_t sig) const noexcept;

        void Unregister(sharpen::FileHandle handle) noexcept;
    };
}   // namespace sharpen

#endif