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
    
        static constexpr std::uint32_t defaultAppendWaitMs_{1*1000};

        static constexpr std::uint32_t defaultElectionMinWaitMs_{3*1000};

        static constexpr std::uint32_t defaultElectionMaxWaitMs_{5*1000};

        std::uint32_t appendWaitMs_;
        std::uint32_t electionMinWaitMs_;
        std::uint32_t electionMaxWaitMs_;
        std::uint32_t randomSeed_;
        TimerMaker timerMaker_;
    public:
        RaftGroupOption()
            :RaftGroupOption(std::random_device{}())
        {}

        explicit RaftGroupOption(std::uint32_t seed) noexcept
            :appendWaitMs_(Self::defaultAppendWaitMs_)
            ,electionMinWaitMs_(Self::defaultElectionMinWaitMs_)
            ,electionMaxWaitMs_(Self::defaultElectionMaxWaitMs_)
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

        inline std::uint32_t GetAppendWaitTime() const noexcept
        {
            return this->appendWaitMs_;
        }

        template<typename _Rep,typename _Period>
        inline void SetAppendWaitTime(const std::chrono::duration<_Rep,_Period> &time) noexcept
        {
            std::uint32_t val{time/std::chrono::milliseconds{1}};
            assert(val != 0);
            this->appendWaitMs_ = val;
        }

        inline std::uint32_t GetMinElectionWaitTime() const noexcept
        {
            return this->electionMinWaitMs_;
        }

        template<typename _Rep,typename _Period>
        inline void SetMinElectionWaitTime(const std::chrono::duration<_Rep,_Period> &time) noexcept
        {
            std::uint32_t val{time/std::chrono::milliseconds{1}};
            assert(val != 0);
            this->electionMinWaitMs_ = val;
        }

        inline std::uint32_t GetMaxElectionWaitTime() const noexcept
        {
            return this->electionMaxWaitMs_;
        }

        template<typename _Rep,typename _Period>
        inline void SetMaxElectionWaitTime(const std::chrono::duration<_Rep,_Period> &time) noexcept
        {
            std::uint32_t val{time/std::chrono::milliseconds{1}};
            assert(val != 0);
            this->electionMaxWaitMs_ = val;
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