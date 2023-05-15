#pragma once
#ifndef _LOGSTEP_HPP
#define _LOGSTEP_HPP

#include <sharpen/IHostPipelineStep.hpp>

class LogStep:public sharpen::IHostPipelineStep {
private:
    using Self = LogStep;

public:
    LogStep() noexcept = default;

    LogStep(const Self &other) noexcept = default;

    LogStep(Self &&other) noexcept = default;

    inline Self &operator=(const Self &other) noexcept {
        if (this != std::addressof(other)) {
            Self tmp{other};
            std::swap(tmp, *this);
        }
        return *this;
    }

    Self &operator=(Self &&other) noexcept = default;

    virtual ~LogStep() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    virtual sharpen::HostPipelineResult Consume(sharpen::INetStreamChannel &channel,
                                                    const std::atomic_bool &active) noexcept override;
};

#endif