#include <sharpen/SimpleHostPipeline.hpp>

sharpen::SimpleHostPipeline::SimpleHostPipeline()
    :token_(true)
    ,pipeline_()
{
    this->pipeline_.reserve(reservedPipelineSize_);
}

sharpen::SimpleHostPipeline::~SimpleHostPipeline() noexcept
{
    this->Stop();
}

void sharpen::SimpleHostPipeline::NviConsume(sharpen::NetStreamChannelPtr channel) noexcept
{
    for(auto begin = this->pipeline_.begin(),end = this->pipeline_.end(); begin != end; ++begin)
    {
        sharpen::IHostPipelineStep *step{begin->get()};
        assert(step);
        sharpen::HostPipelineResult result{step->Consume(*channel,this->token_)};
        if(result == sharpen::HostPipelineResult::Broken)
        {
            break;
        }   
    }
}

void sharpen::SimpleHostPipeline::NviRegisterStep(std::unique_ptr<sharpen::IHostPipelineStep> step)
{
    this->pipeline_.emplace_back(std::move(step));
}