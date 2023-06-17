#pragma once
#ifndef _SHARPEN_COMMONID_HPP
#define _SHARPEN_COMMONID_HPP

#include <utility>

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
    };
#pragma pack(pop)

    static_assert(sizeof(CommonId) == CommonIdSize,"sizeof(CommonId) should be CommonIdSize");
}   // namespace sharpen

#endif