#pragma once
#ifndef _SHARPEN_ICONSENSUSADVANCER_HPP
#define _SHARPEN_ICONSENSUSADVANCER_HPP

#include "AwaitableFuture.hpp"
#include "ConsensusTask.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>



namespace sharpen {
    class IConsensusAdvancer {
    private:
        using Self = sharpen::IConsensusAdvancer;

    protected:
        virtual void NviWaitAsync(sharpen::ConsensusTask task) = 0;

    public:
        IConsensusAdvancer() noexcept = default;

        IConsensusAdvancer(const Self &other) noexcept = default;

        IConsensusAdvancer(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IConsensusAdvancer() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void Run() = 0;

        virtual void Stop() noexcept = 0;

        inline void WaitAsync(std::uint64_t index,
                              sharpen::Future<sharpen::ConsensusStatus> &future) {
            assert(index != 0);
            sharpen::ConsensusTask task{index, future};
            assert(task.Vaild());
            if (task.Vaild()) {
                this->NviWaitAsync(std::move(task));
            }
        }

        inline sharpen::ConsensusStatus WaitAsync(std::uint64_t index) {
            sharpen::AwaitableFuture<sharpen::ConsensusStatus> future;
            this->WaitAsync(index, future);
            return future.Await();
        }
    };
}   // namespace sharpen

#endif