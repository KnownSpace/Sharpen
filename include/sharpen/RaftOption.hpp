#pragma once
#ifndef _SHARPEN_RAFTOPTION_HPP
#define _SHARPEN_RAFTOPTION_HPP

#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class RaftOption {
    private:
        using Self = sharpen::RaftOption;

        static constexpr std::uint32_t maxBatchSize_{16 * 1024};

        static constexpr std::uint32_t minBatchSize_{1};

        static constexpr std::uint32_t maxPipelineLength_{32};

        static constexpr std::uint32_t minPipelineLength_{1};

        bool isLearner_;
        bool enablePrevote_;
        std::uint32_t batchSize_;
        std::uint32_t pipelineLength_;
        bool enableSingle_;
        bool enableLeaseAwareness_; 

    public:
        RaftOption() noexcept;

        RaftOption(const Self &other) noexcept = default;

        RaftOption(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftOption() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline bool IsLearner() const noexcept {
            return this->isLearner_;
        }

        inline void SetLearner(bool learner) noexcept {
            this->isLearner_ = learner;
        }

        inline bool IsEnablePrevote() const noexcept {
            return this->enablePrevote_;
        }

        inline void SetPrevote(bool prevote) noexcept {
            this->enablePrevote_ = prevote;
        }

        inline void EnablePrevote() noexcept {
            this->SetPrevote(true);
        }

        inline void DisablePrevote() noexcept {
            this->SetPrevote(false);
        }

        inline std::uint32_t GetBatchSize() const noexcept {
            return this->batchSize_;
        }

        void SetBatchSize(std::uint32_t batchSize) noexcept;

        inline std::uint32_t GetPipelineLength() noexcept {
            return this->pipelineLength_;
        }
        
        void SetPipelineLength(std::uint32_t pipeline) noexcept;

        inline bool IsEnableSingle() const noexcept {
            return this->enableSingle_;
        }

        inline void SetSingle(bool single) noexcept {
            this->enableSingle_ = single;
        }

        inline void EnableSingle() noexcept {
            this->SetSingle(true);
        }

        inline void DisableSingle() noexcept {
            this->SetSingle(false);
        }

        inline bool IsEnableLeaseAwareness() const noexcept {
            return this->enableLeaseAwareness_;
        }

        inline void SetLeaseAwareness(bool leaseAwareness) noexcept {
            this->enableLeaseAwareness_ = leaseAwareness;
        }

        inline void EnableLeaseAwareness() noexcept {
            this->SetLeaseAwareness(true);
        }

        inline void DisableLeaseAwareness() noexcept {
            this->SetLeaseAwareness(false);
        }
    };
}   // namespace sharpen

#endif