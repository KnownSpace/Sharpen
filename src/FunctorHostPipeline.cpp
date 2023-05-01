#include <sharpen/FunctorHostPipeline.hpp>

#include <sharpen/NoexceptInvoke.hpp>

sharpen::FunctorHostPipeline::FunctorHostPipeline()
    : token_(true)
    , pipeline_()
    , functors_() {
    this->pipeline_.reserve(reservedPipelineSize_);
    this->functors_.reserve(reservedPipelineSize_);
}

sharpen::FunctorHostPipeline::~FunctorHostPipeline() noexcept {
    this->token_ = false;
}

void sharpen::FunctorHostPipeline::NviConsume(sharpen::NetStreamChannelPtr channel) noexcept {
    for (auto begin = this->functors_.begin(), end = this->functors_.end(); begin != end; ++begin) {
        assert(*begin);
        sharpen::HostPipelineResult result{
            sharpen::NonexceptInvoke(*begin, std::ref(*channel), std::cref(this->token_))};
        if (result == sharpen::HostPipelineResult::Broken) {
            return;
        }
    }
}

void sharpen::FunctorHostPipeline::NviRegister(std::unique_ptr<sharpen::IHostPipelineStep> step) {
    sharpen::IHostPipelineStep *rawStep{step.get()};
    this->pipeline_.emplace_back(std::move(step));
    this->functors_.emplace_back(std::bind(
        &IHostPipelineStep::Consume, rawStep, std::placeholders::_1, std::placeholders::_2));
}

sharpen::FunctorHostPipeline &sharpen::FunctorHostPipeline::Register(Functor func) {
    this->functors_.emplace_back(std::move(func));
    return *this;
}