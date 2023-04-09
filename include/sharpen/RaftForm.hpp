#pragma once
#ifndef _SHARPEN_RAFTFORMS_HPP
#define _SHARPEN_RAFTFORMS_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#include "RaftMailType.hpp"
#include "ByteSlice.hpp"

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

        //string "raft" copy to a uint32_t
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

        inline sharpen::RaftMailType GetType() const noexcept
        {
            if(sharpen::IsValiedRaftMailType(this->type_))
            {
                return static_cast<sharpen::RaftMailType>(this->type_);
            }
            return sharpen::RaftMailType::Unknown;
        }

        inline void SetType(sharpen::RaftMailType type) noexcept
        {
            this->type_ = static_cast<std::uint32_t>(type);
        }

        inline std::uint16_t GetChecksum() const noexcept
        {
            return this->chksum_;
        }

        inline void SetChecksum(std::uint16_t chksum) noexcept
        {
            this->chksum_ = chksum;
        }

        void SetChecksum(sharpen::ByteSlice slice) noexcept;

        void SetChecksum(const char *data,std::size_t size) noexcept;

        inline std::uint32_t GetMagic() const noexcept
        {
            return this->magic_;
        }

        inline bool CheckMagic() const noexcept
        {
            return this->magic_ == raftMagic;
        }

        bool CheckContent(sharpen::ByteSlice slice) const noexcept;
    };
    #pragma pack(pop)

    static_assert(sizeof(sharpen::RaftForm) == 16,"sizeof RaftForm must equals with 16");
}

#endif