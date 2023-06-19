#pragma once
#ifndef _SHARPEN_ACTORID_HPP
#define _SHARPEN_ACTORID_HPP

#include "CommonId.hpp"

namespace sharpen {
#pragma pack(push, 1)
    class ActorId : public sharpen::CommonId {
    private:
        using Self = sharpen::ActorId;

    public:
        ActorId() noexcept = default;

        ActorId(const Self &other) noexcept = default;

        ActorId(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        ~ActorId() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline bool operator==(const Self &other) const noexcept {
            return this->CompareWith(other) == 0;
        }

        inline bool operator!=(const Self &other) const noexcept {
            return this->CompareWith(other) != 0;
        }

        inline bool operator<(const Self &other) const noexcept {
            return this->CompareWith(other) < 0;
        }

        inline bool operator>(const Self &other) const noexcept {
            return this->CompareWith(other) > 0;
        }

        inline bool operator>=(const Self &other) const noexcept {
            return this->CompareWith(other) >= 0;
        }

        inline bool operator<=(const Self &other) const noexcept {
            return this->CompareWith(other) <= 0;
        }
    };
#pragma pack(pop)
}   // namespace sharpen

namespace std {
    template<typename _T>
    struct hash;

    template<>
    struct hash<sharpen::ActorId> {
        std::size_t operator()(const sharpen::ActorId &id) const noexcept {
            return hash<sharpen::CommonId>{}(id);
        }
    };
}   // namespace std

#endif