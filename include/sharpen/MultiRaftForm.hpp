#pragma once
#ifndef _SHARPEN_MULTIRAFTFORM_HPP
#define _SHARPEN_MULTIRAFTFORM_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#include "RaftMailType.hpp"
#include "ByteSlice.hpp"

namespace sharpen
{
    class MultiRaftForm
    {
    private:
        using Self = sharpen::MultiRaftForm;
    
        char magic_[6];
        std::uint32_t type_;
        std::uint16_t chksum_;
        std::uint32_t number_;
    public:
    
        static constexpr sharpen::ByteSlice multiRaftMagic{"mlraft",6};

        MultiRaftForm() noexcept;

        explicit MultiRaftForm(sharpen::RaftMailType type) noexcept;

        MultiRaftForm(sharpen::RaftMailType type,std::uint32_t number) noexcept;
    
        MultiRaftForm(const Self &other) noexcept;
    
        MultiRaftForm(Self &&other) noexcept;
    
        Self &operator=(const Self &other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~MultiRaftForm() noexcept = default;
    
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

        inline sharpen::ByteSlice GetMagic() const noexcept
        {
            sharpen::ByteSlice slice{this->magic_,sizeof(this->magic_)};
            return slice;
        }

        inline bool CheckMagic() const noexcept
        {
            return this->GetMagic() == multiRaftMagic;
        }

        inline std::uint32_t GetRaftNumber() const noexcept
        {
            return this->number_;
        }

        inline void SetRaftNumber(std::uint32_t number) noexcept
        {
            this->number_ = number;
        }

        bool CheckContent(sharpen::ByteSlice content) const noexcept;
    };
}

#endif