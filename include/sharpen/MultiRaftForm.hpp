#pragma once
#ifndef _SHARPEN_MULTIRAFTFORM_HPP
#define _SHARPEN_MULTIRAFTFORM_HPP

#include "ByteSlice.hpp"
#include "RaftMailType.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
#pragma pack(push, 1)
    class MultiRaftForm {
    private:
        using Self = sharpen::MultiRaftForm;

        char magic_[6];
        std::uint32_t type_;
        std::uint16_t chksum_;
        std::uint32_t number_;

    public:
        static constexpr sharpen::ByteSlice multiRaftMagic{"mlraft", 6};

        MultiRaftForm() noexcept;

        explicit MultiRaftForm(sharpen::RaftMailType type) noexcept;

        MultiRaftForm(sharpen::RaftMailType type, std::uint32_t number) noexcept;

        MultiRaftForm(const Self &other) noexcept;

        MultiRaftForm(Self &&other) noexcept;

        Self &operator=(const Self &other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~MultiRaftForm() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        sharpen::RaftMailType GetType() const noexcept;

        void SetType(sharpen::RaftMailType type) noexcept;

        std::uint16_t GetChecksum() const noexcept;

        void SetChecksum(std::uint16_t chksum) noexcept;

        void SetChecksum(sharpen::ByteSlice slice) noexcept;

        void SetChecksum(const char *data, std::size_t size) noexcept;

        inline sharpen::ByteSlice GetMagic() const noexcept {
            sharpen::ByteSlice slice{this->magic_, sizeof(this->magic_)};
            return slice;
        }

        inline bool CheckMagic() const noexcept {
            return this->GetMagic() == multiRaftMagic;
        }

        std::uint32_t GetRaftNumber() const noexcept;

        void SetRaftNumber(std::uint32_t number) noexcept;

        bool CheckContent(sharpen::ByteSlice content) const noexcept;
    };

#pragma pack(pop)

    static_assert(sizeof(sharpen::MultiRaftForm) == 16,
                  "sizeof(MultiRaftForm) must equals with 16");
}   // namespace sharpen

#endif