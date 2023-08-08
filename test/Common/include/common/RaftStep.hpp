#pragma once
#ifndef _RAFTSTEP_HPP
#define _RAFTSTEP_HPP

#include <sharpen/IHostPipelineStep.hpp>
#include <sharpen/GenericMail.hpp>
#include <sharpen/GenericMailParser.hpp>
#include <sharpen/GenericMailParserFactory.hpp>
#include <sharpen/IConsensus.hpp>

class RaftStep:public sharpen::IHostPipelineStep {
private:
    using Self = RaftStep;

    std::unique_ptr<sharpen::IMailParserFactory> factory_;
    std::shared_ptr<sharpen::IConsensus> raft_;
    bool logging_;

public:
    RaftStep(std::uint32_t magicNumber,std::shared_ptr<sharpen::IConsensus> raft) noexcept;

    RaftStep(Self &&other) noexcept = default;

    Self &operator=(Self &&other) noexcept = default;

    virtual ~RaftStep() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline void DisableLogging() noexcept {
        this->logging_ = false;
    }

    virtual sharpen::HostPipelineResult Consume(sharpen::INetStreamChannel &channel,
                                                    const std::atomic_bool &active) noexcept override;
};

#endif