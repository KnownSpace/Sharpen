#pragma once
#ifndef _SHARPEN_RAFTOPTION_HPP
#define _SHARPEN_RAFTOPTION_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

namespace sharpen
{
    class RaftOption
    {
    private:
        using Self = sharpen::RaftOption;
    
        bool isLearner_;
        bool enablePrevote_;
    public:
    
        RaftOption() noexcept;
    
        RaftOption(const Self &other) noexcept = default;
    
        RaftOption(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other) noexcept
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~RaftOption() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline bool IsLearner() const noexcept
        {
            return this->isLearner_;
        }

        inline void SetLearner(bool learner) noexcept
        {
            this->isLearner_ = learner;
        }

        inline bool EnablePrevote() const noexcept
        {
            return this->enablePrevote_;
        }

        inline void SetPrevote(bool prevote) noexcept
        {
            this->enablePrevote_ = prevote;
        }
    };
}

#endif