#pragma once
#ifndef _HANDLESTEP_HPP
#define _HANDLESTEP_HPP

#include <sharpen/IHostPipelineStep.hpp>
#include <sharpen/GenericMail.hpp>
#include <sharpen/GenericMailParser.hpp>
#include <sharpen/GenericMailParserFactory.hpp>

class HandleStep : public sharpen::IHostPipelineStep,public sharpen::Noncopyable {
private:
    using Self = HandleStep;

    std::unique_ptr<sharpen::IMailParserFactory> factory_;
    std::function<void(sharpen::INetStreamChannel*,sharpen::Mail)> handler_;
public:
    HandleStep(std::uint32_t magic,std::function<void(sharpen::INetStreamChannel*,sharpen::Mail)> handler);

    HandleStep(Self &&other) noexcept = default;

    Self &operator=(Self &&other) noexcept;

    virtual ~HandleStep() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    virtual sharpen::HostPipelineResult Consume(sharpen::INetStreamChannel &channel,
                                                const std::atomic_bool &active) noexcept override;
};

#endif