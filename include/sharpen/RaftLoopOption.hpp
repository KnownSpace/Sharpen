#pragma once
#ifndef _SHARPEN_RAFTGROUPOPTION_HPP
#define _SHARPEN_RAFTGROUPOPTION_HPP

#include <chrono>
#include <cassert>
#include <random>
#include <cstdint>
#include <cstddef>

#include "ITimer.hpp"

namespace sharpen
{
    class RaftLoopOption
    {
    private:
        using Self = sharpen::RaftLoopOption;
        using TimerMaker = sharpen::TimerPtr(*)(sharpen::EventEngine&);
    
        static constexpr std::uint32_t defaultAppendEntriesCycle_{1*1000};

        static constexpr std::uint32_t defaultMinElectionCycle_{3*1000};

        static constexpr std::uint32_t defaultMaxElectionCycle_{5*1000};

        std::chrono::milliseconds appendEntriesCycle_;
        std::chrono::milliseconds minElectionCycle_;
        std::chrono::milliseconds maxElectionCycle_;
        std::uint32_t randomSeed_;
        TimerMaker timerMaker_;
    public:
        RaftLoopOption()
            :RaftLoopOption(std::random_device{}())
        {}

        explicit RaftLoopOption(std::uint32_t seed) noexcept
            :appendEntriesCycle_(static_cast<std::int64_t>(Self::defaultAppendEntriesCycle_))
            ,minElectionCycle_(static_cast<std::int64_t>(Self::defaultMinElectionCycle_))
            ,maxElectionCycle_(static_cast<std::int64_t>(Self::defaultMaxElectionCycle_))
            ,randomSeed_(seed)
            ,timerMaker_(nullptr)
        {}
    
        RaftLoopOption(const Self &other) = default;
    
        RaftLoopOption(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            return *this;
        }
    
        ~RaftLoopOption() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::chrono::milliseconds GetAppendEntriesCycle() const noexcept
        {
            return this->appendEntriesCycle_;
        }

        template<typename _Rep,typename _Period>
        inline void SetAppendEntriesCycle(const std::chrono::duration<_Rep,_Period> &time) noexcept
        {
            assert(time/std::chrono::milliseconds(1) != 0);
            this->appendEntriesCycle_ = time;
        }

        inline std::chrono::milliseconds GetMinElectionCycle() const noexcept
        {
            return this->minElectionCycle_;
        }

        template<typename _Rep,typename _Period>
        inline void SetMinElectionCycle(const std::chrono::duration<_Rep,_Period> &time) noexcept
        {
            assert(time/std::chrono::milliseconds(1) != 0);
            this->minElectionCycle_ = time;
        }

        inline std::chrono::milliseconds GetMaxElectionCycle() const noexcept
        {
            return this->maxElectionCycle_;
        }

        template<typename _Rep,typename _Period>
        inline void SetMaxElectionCycle(const std::chrono::duration<_Rep,_Period> &time) noexcept
        {
            assert(time/std::chrono::milliseconds(1) != 0);
            this->maxElectionCycle_ = time;
        }

        inline std::uint32_t GetRandomSeed() const noexcept
        {
            return this->randomSeed_;
        }

        inline void SetRandomSeed(std::uint32_t seed) noexcept
        {
            this->randomSeed_ = seed;
        }

        inline TimerMaker GetTimerMaker() const noexcept
        {
            if(!this->timerMaker_)
            {
                return &sharpen::MakeTimer;
            }
            return this->timerMaker_;
        }

        inline void SetTimerMaker(TimerMaker maker) noexcept
        {
            this->timerMaker_ = maker;
        }
    };
}

#endif