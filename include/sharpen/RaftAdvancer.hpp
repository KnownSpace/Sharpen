#pragma once
#ifndef _SHARPEN_RAFTADVANCER_HPP
#define _SHARPEN_RAFTADVANCER_HPP

#include "IConsensusAdvancer.hpp"
#include "ITimer.hpp"
#include "Noncopyable.hpp"
#include "RaftConsensus.hpp"
#include <queue>



namespace sharpen {
    class RaftAdvancer
        : public sharpen::IConsensusAdvancer
        , public sharpen::Noncopyable {
    private:
        using Self = sharpen::RaftAdvancer;

        std::priority_queue<sharpen::ConsensusTask> taskQueue_;
        std::atomic_bool status_;
        sharpen::TimerPtr timer_;
        sharpen::RaftConsensus *raft_;

        virtual void NviWaitAsync(sharpen::ConsensusTask task) override;

    public:
        RaftAdvancer(sharpen::RaftConsensus &raft);

        RaftAdvancer(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        virtual ~RaftAdvancer() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void Run() override;

        virtual void Stop() noexcept override;
    };
}   // namespace sharpen

#endif