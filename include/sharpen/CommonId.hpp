#pragma once
#ifndef _SHARPEN_COMMONID_HPP
#define _SHARPEN_COMMONID_HPP

#include <utility>
#include <type_traits>

namespace sharpen {
#pragma pack(push, 1)
    constexpr std::size_t CommonIdSize{24};
    class CommonId {
    private:
        using Self = sharpen::CommonId;

        char data_[CommonIdSize];
    public:
        CommonId() noexcept;

        CommonId(const Self &other) noexcept;

        CommonId(Self &&other) noexcept;

        Self &operator=(const Self &other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~CommonId() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        char *Data() noexcept;

        const char *Data() const noexcept;

        void Zero() noexcept;

        bool operator==(const Self &other) const noexcept;

        inline bool operator!=(const Self &other) const noexcept {
            return !(*this == other);
        }
    };
#pragma pack(pop)

    static_assert(sizeof(sharpen::CommonId) == CommonIdSize,"sizeof(sharpen::CommonId) should be CommonIdSize");

    static_assert(std::is_standard_layout<sharpen::CommonId>::value,"sharpen::CommonId should be standard layout");
}   // namespace sharpen

#endif