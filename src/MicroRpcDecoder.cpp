#include <sharpen/MicroRpcDecoder.hpp>

#include <sharpen/CompilerInfo.hpp>
#include <sharpen/ForceInline.hpp>

sharpen::MicroRpcDecoder::MicroRpcDecoder() noexcept
    :stack_(nullptr)
    ,begin_(0)
    ,end_(0)
    ,step_(Step::WaitMetadata)
    ,completed_(false)
    ,error_(nullptr)
{}

sharpen::MicroRpcDecoder::MicroRpcDecoder(Self &&other) noexcept
    :stack_(other.stack_)
    ,begin_(other.begin_)
    ,end_(other.end_)
    ,step_(other.step_)
{
    other.stack_ = nullptr;
    other.begin_ = 0;
    other.end_ = 0;
    other.step_ = Step::WaitMetadata;
}

sharpen::MicroRpcDecoder &sharpen::MicroRpcDecoder::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->stack_ = other.stack_;
        this->begin_ = other.begin_;
        this->end_ = other.end_;
        this->step_ = other.step_;
        other.stack_ = nullptr;
        other.begin_ = 0;
        other.end_ = 0;
        other.step_ = Step::WaitMetadata;
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
            this->stack_->Push();
            this->stack_->Top().RawData().PushBack(*begin);
            ++begin;
            if (this->stack_->Top().Header().type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Void))
            {
                this->step_ = Step::Completed;
                return begin;
            }
            this->end_ = this->stack_->Top().Header().sizeSpace_;
            if (this->end_ == 7)
            {
                this->end_ += 1;
            }
            this->begin_ = 0;
            this->stack_->Top().RawData().Reserve(this->end_ + sharpen::GetMicroRpcTypeSize(static_cast<sharpen::MicroRpcVariableType>(this->stack_->Top().Header().type_)));
            if (this->end_ == 0)
            {
                this->step_ = Step::WaitData;
                this->end_ = 1;
                goto WaitDataLab;
            }
            this->step_ = Step::WaitSize;
    case Step::WaitSize:
            while (this->begin_ != this->end_)
            {
                this->stack_->Top().RawData().PushBack(*begin);
                ++begin;
                ++this->begin_;
                if(begin != end)
                {
                    continue;
                }
                return begin;
            }
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::ConvertEndian(this->stack_.Top().RawData().Data() + 1, this->end_);
#endif
            this->step_ = Step::WaitData;
            {
                std::memcpy(&this->end_, this->stack_->Top().RawData().Data() + 1, this->end_);
                this->begin_ = 0;
                this->stack_->Top().RawData().Reserve(this->end_);
            }
    case Step::WaitData:
    WaitDataLab:
            {
                sharpen::Size record{0};
                sharpen::Size typeSize{sharpen::GetMicroRpcTypeSize(static_cast<sharpen::MicroRpcVariableType>(this->stack_->Top().Header().type_))};
                while (begin != end)
                {
                    this->stack_->Top().RawData().PushBack(*begin);
                    ++begin;
                    if(++record == typeSize)
                    {
                        record = 0;
#ifdef SHARPEN_IS_BIG_ENDIAN
                sharpen::ConvertEndian(&*this->stack_.Top().RawData().End() - typeSize,typeSize);
#endif
                        if(++this->begin_ == this->end_)
                        {
                            this->step_ = Step::Completed;
                            return begin;
                        }
                    }
                }
                return begin;
            }
        case Step::Completed:
        case Step::Error:
            return begin;
        }
    }
    return nullptr;
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
    while (begin != end)
    {
        begin = this->RunStateMachine(begin,end);
        if(this->step_ == Step::Completed)
        {
            if(this->stack_->Top().Header().end_)
            {
                //set completed
                this->completed_ = true;
                this->stack_->Reverse();
                break;
            }
            this->step_ = Step::WaitMetadata;
        }
    }
    return begin - data;
}