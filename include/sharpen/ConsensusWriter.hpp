#pragma once
#ifndef _SHARPEN_CONSENSUSWRITERID_HPP
#define _SHARPEN_CONSENSUSWRITERID_HPP

#include "ActorId.hpp"
#include <cstdint>

namespace sharpen {
    class ConsensusWriter {
    private:
        using Self = sharpen::ConsensusWriter;

        sharpen::ActorId id_;
        std::uint64_t epoch_;

    public:
        static constexpr std::uint64_t noneEpoch{0};

        ConsensusWriter() noexcept = default;

        ConsensusWriter(const Self &other) noexcept = default;

        ConsensusWriter(Self &&other) noexcept = default;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept = default;

        ~ConsensusWriter() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline sharpen::ActorId &WriterId() noexcept
        {
            return this->id_;
        }
        
        inline const sharpen::ActorId &WriterId() const noexcept
        {
            return this->id_;
        }

        inline std::uint64_t GetEpoch() const noexcept {
            return this->epoch_;
        }

        inline void SetEpoch(std::uint64_t epoch) noexcept {
            this->epoch_ = epoch;
        }

        inline bool Valid() const noexcept {
            return this->epoch_ != noneEpoch;
        }
    };
}   // namespace sharpen

#endif