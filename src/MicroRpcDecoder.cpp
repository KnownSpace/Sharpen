#include <sharpen/MicroRpcDecoder.hpp>

#include <sharpen/CompilerInfo.hpp>
#include <sharpen/ForceInline.hpp>

sharpen::MicroRpcDecoder::MicroRpcDecoder() noexcept
    :stack_(nullptr)
    ,ite_(0)
    ,step_(Step::WaitMetadata)
    ,typeSize_(0)
    ,record_(0)
    ,completed_(false)
{}

sharpen::MicroRpcDecoder::MicroRpcDecoder(Self &&other) noexcept
    :stack_(other.stack_)
    ,ite_(other.ite_)
    ,step_(other.step_)
    ,typeSize_(other.typeSize_)
    ,record_(other.record_)
    ,completed_(other.completed_)
{
    other.stack_ = nullptr;
    other.ite_ = 0;
    other.step_ = Step::WaitMetadata;
    other.typeSize_ = 0;
    other.record_ = 0;
    other.completed_ = false;
}

sharpen::MicroRpcDecoder &sharpen::MicroRpcDecoder::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->stack_ = other.stack_;
        this->ite_ = other.ite_;
        this->step_ = other.step_;
        this->typeSize_ = other.typeSize_;
        this->record_ = other.record_;
        this->completed_ = other.completed_;
        other.stack_ = nullptr;
        other.ite_ = 0;
        other.step_ = Step::WaitMetadata;
        other.typeSize_ = 0;
        other.record_ = 0;
        other.completed_ = false;
    }
    return *this;
}

//[fallthrough]
#ifdef SHARPEN_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#elif (defined SHARPEN_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:26819)
#elif (defined SHARPEN_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#endif

const char *sharpen::MicroRpcDecoder::RunStateMachine(const char *begin,const char *end)
{
    assert(this->stack_);
    while(begin != end)
    {
        switch (this->step_)
        {
        case Step::WaitMetadata:
        WaitMetadataLab:
            this->stack_->Push();
            this->stack_->Top().RawData().Reserve(16);
            this->stack_->Top().RawData().PushBack(*begin);
            ++begin;
            if (this->stack_->Top().Header().type_ > 10)
            {
                throw sharpen::MicroRpcParseException("unknown type");
            }
            if (this->stack_->Top().Header().type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Void))
            {
                this->step_ = Step::Completed;
                goto CompletedLab;
            }
            this->typeSize_ = sharpen::GetMicroRpcTypeSize(static_cast<sharpen::MicroRpcVariableType>(this->stack_->Top().Header().type_));
            this->record_ = 0;
            this->ite_ = this->stack_->Top().Header().sizeSpace_;
            if (this->ite_ == 0)
            {
                this->step_ = Step::WaitData;
                this->ite_ = 1;
                goto WaitDataLab;
            }
            this->ite_ = this->ite_ == 7 ? 8: this->ite_;
            if(this->ite_ > sizeof(sharpen::Size))
            {
                throw sharpen::MicroRpcParseException("message too large");
            }
            this->step_ = Step::WaitSize;
        case Step::WaitSize:
            if (this->ite_)
            {
                this->stack_->Top().RawData().PushBack(*begin);
                ++begin;
                --this->ite_;
                continue;
            }
            this->step_ = Step::WaitData;
            std::memcpy(&this->ite_, this->stack_->Top().RawData().Data() + 1, this->stack_->Top().RawData().GetSize() - 1);
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::ConvertEndian(this->stack_.Top().RawData().Data() + 1, this->ite_);
#endif
            this->stack_->Top().RawData().Reserve(this->ite_ * this->typeSize_);
        case Step::WaitData:
        WaitDataLab:
            this->stack_->Top().RawData().PushBack(*begin);
            ++begin;
            if(this->typeSize_ == 1 || (++this->record_ == this->typeSize_ && (this->record_ = 0,true)))
            {
                if(!--this->ite_)
                {
                    this->step_ = Step::Completed;
                    goto CompletedLab;
                }
#ifdef SHARPEN_IS_BIG_ENDIAN
                sharpen::ConvertEndian(&*this->stack_.Top().RawData().End() - this->typeSize_,this->typeSize_);
#endif
            }
            continue;
        case Step::Completed:
        CompletedLab:
            this->step_ = Step::WaitMetadata;
            if(this->stack_->Top().Header().end_)
            {
                this->stack_->Reverse();
                this->completed_ = true;
                return begin;
            }
            goto WaitMetadataLab;
        }
    }
    return begin;
}

#ifdef SHARPEN_COMPILER_GCC
#pragma GCC diagnostic push
#elif (defined SHARPEN_COMPILER_MSVC)
#pragma warning(pop)
#elif (defined SHARPEN_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

sharpen::Size sharpen::MicroRpcDecoder::Decode(const char *data, sharpen::Size size)
{
    assert(this->stack_);
    const char *begin = data;
    const char *end = data + size;
    begin = this->RunStateMachine(begin,end);
    return begin - data;
}