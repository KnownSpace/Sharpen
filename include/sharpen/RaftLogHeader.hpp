#pragma once
#ifndef _SHARPEN_RAFTLOGHEADER_HPP
#define _SHARPEN_RAFTLOGHEADER_HPP

#include <cstdint>
#include <utility>


namespace sharpen {
#pragma pack(push, 1)
    class RaftLogHeader {
    private:
        using Self = sharpen::RaftLogHeader;

        std::uint32_t magic_;
        std::uint32_t checksum_;
        std::uint64_t term_;

    public:
        RaftLogHeader() noexcept;

        explicit RaftLogHeader(std::uint32_t magic,
                               std::uint32_t checksum,
                               std::uint64_t term) noexcept;

        RaftLogHeader(const Self &other) noexcept = default;

        RaftLogHeader(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftLogHeader() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        std::uint32_t GetMagic() const noexcept;

        void SetMagic(std::uint32_t magic) noexcept;

        std::uint32_t GetChecksum() const noexcept;

        void SetChecksum(std::uint32_t checksum) noexcept;

        std::uint64_t GetTerm() const noexcept;

        void SetTerm(std::uint64_t term) noexcept;
    };
#pragma pack(pop)
}   // namespace sharpen

#endif