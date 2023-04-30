#pragma once
#ifndef _SHARPEN_RAFTFORMS_HPP
#define _SHARPEN_RAFTFORMS_HPP

#include "ByteSlice.hpp"
#include "RaftMailType.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen
{
#pragma pack(push, 1)
    class RaftForm
    {
    private:
        using Self = sharpen::RaftForm;

        std::uint32_t magic_;
        std::uint32_t type_;
        std::uint16_t chksum_;
        char padding_[6];

    public:
        // string "raft" copy to a uint32_t
        constexpr static std::uint32_t raftMagic{1952866674};

        RaftForm() noexcept;

        explicit RaftForm(sharpen::RaftMailType type) noexcept;

        RaftForm(const Self &other) noexcept;

        RaftForm(Self &&other) noexcept;

        Self &operator=(const Self &other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~RaftForm() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        sharpen::RaftMailType GetType() const noexcept;

        void SetType(sharpen::RaftMailType type) noexcept;

        std::uint16_t GetChecksum() const noexcept;

        void SetChecksum(std::uint16_t chksum) noexcept;

        void SetChecksum(sharpen::ByteSlice slice) noexcept;

        void SetChecksum(const char *data, std::size_t size) noexcept;

        std::uint32_t GetMagic() const noexcept;

        bool CheckMagic() const noexcept;

        bool CheckContent(sharpen::ByteSlice slice) const noexcept;
    };
#pragma pack(pop)

    static_assert(sizeof(sharpen::RaftForm) == 16, "sizeof RaftForm must equals with 16");
}   // namespace sharpen

#endif