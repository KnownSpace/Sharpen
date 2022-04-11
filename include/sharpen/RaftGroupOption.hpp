#pragma once
#ifndef _SHARPEN_RAFTGROUPOPTION_HPP
#define _SHARPEN_RAFTGROUPOPTION_HPP

#include <chrono>
#include <cassert>
#include <random>

#include "TypeDef.hpp"
#include "ITimer.hpp"

namespace sharpen
{
    class RaftGroupOption
    {
    private:
        using Self = sharpen::RaftGroupOption;
        using TimerMaker = sharpen::TimerPtr(*)(sharpen::EventEngine&);
    
        static constexpr sharpen::Uint32 defaultAppendEntriesCycle_{1*1000};

        static constexpr sharpen::Uint32 defaultMinElectionCycle_{3*1000};

        static constexpr sharpen::Uint32 defaultMaxElectionCycle_{5*1000};

        std::chrono::milliseconds appendEntriesCycle_;
        std::chrono::milliseconds minElectionCycle_;
        std::chrono::milliseconds maxElectionCycle_;
        sharpen::Uint32 randomSeed_;
        TimerMaker timerMaker_;
    public:
        RaftGroupOption()
            :RaftGroupOption(std::random_device{}())
        {}

        explicit RaftGroupOption(sharpen::Uint32 seed) noexcept
            :appendEntriesCycle_(Self::defaultAppendEntriesCycle_)
            ,minElectionCycle_(Self::defaultMinElectionCycle_)
            ,maxElectionCycle_(Self::defaultMaxElectionCycle_)
            ,randomSeed_(seed)
            ,timerMaker_(nullptr)
        {}
    
        RaftGroupOption(const Self &other) = default;
    
        RaftGroupOption(Self &&other) noexcept = default;
    
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
    
        ~RaftGroupOption() noexcept = default;
    
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

        inline sharpen::Uint32 GetRandomSeed() const noexcept
        {
            return this->randomSeed_;
        }

        inline void SetRandomSeed(sharpen::Uint32 seed) noexcept
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