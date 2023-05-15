#pragma once
#ifndef _DROPSTEP_HPP
#define _DROPSTEP_HPP

#include <sharpen/IHostPipelineStep.hpp>

class DropStep : public sharpen::IHostPipelineStep {
private:
    using Self = DropStep;

public:
    DropStep() noexcept = default;

    DropStep(const Self &other) noexcept = default;

    DropStep(Self &&other) noexcept = default;

    inline Self &operator=(const Self &other) noexcept {
        if (this != std::addressof(other)) {
            Self tmp{other};
            std::swap(tmp, *this);
        }
        return *this;
    }

    Self &operator=(Self &&other) noexcept = default;

    ~DropStep() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    virtual sharpen::HostPipelineResult Consume(sharpen::INetStreamChannel &channel,
                                                const std::atomic_bool &active) noexcept override;
};

#endif