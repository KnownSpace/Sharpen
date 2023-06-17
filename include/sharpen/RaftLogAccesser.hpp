#pragma once
#ifndef _SHARPEN_RAFTLOGACCESSER_HPP
#define _SHARPEN_RAFTLOGACCESSER_HPP

#include "IRaftLogAccesser.hpp"
#include "RaftLogHeader.hpp" // IWYU pragma: export

namespace sharpen {
    class RaftLogAccesser : public sharpen::IRaftLogAccesser {
    private:
        using Self = sharpen::RaftLogAccesser;

        std::uint32_t magic_;

        virtual std::uint64_t NviGetTerm(sharpen::ByteSlice logEntry) const noexcept override;

        virtual bool NviIsRaftEntry(sharpen::ByteSlice logEntry) const noexcept override;

        virtual sharpen::ByteBuffer NviCreateEntry(sharpen::ByteSlice bytes,
                                                   std::uint64_t term) const override;

    public:
        RaftLogAccesser(std::uint32_t magic) noexcept;

        RaftLogAccesser(const Self &other) noexcept = default;

        RaftLogAccesser(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual ~RaftLogAccesser() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }
    };
}   // namespace sharpen

#endif