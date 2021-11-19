#include <sharpen/MicroRpcDecoder.hpp>

#include <sharpen/CompilerInfo.hpp>

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

#ifdef SHARPEN_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#elif (defined SHARPEN_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:4507)
#else
#endif

void sharpen::MicroRpcDecoder::RunStateMachine(char data)
{
    assert(this->stack_);
    switch (this->step_)
    {
    case Step::WaitMetadata:
        this->stack_->Push();
        this->stack_->Top().RawData().PushBack(data);
        if (this->stack_->Top().Header().type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Void))
        {
            this->step_ = Step::Completed;
            return;
        }
        this->end_ = this->stack_->Top().Header().sizeSpace_;
        if (this->end_ == 7)
        {
            this->end_ += 1;
        }
        this->begin_ = 0;
        this->stack_->Top().RawData().Reserve(this->end_ + sharpen::GetMicroRpcTypeSize(static_cast<sharpen::MicroRpcVariableType>(this->stack_->Top().Header().type_)));
        if (this->end_ != 0)
        {
            this->step_ = Step::WaitSize;
            return;
        }
        this->step_ = Step::WaitData;
        this->end_ = sharpen::GetMicroRpcTypeSize(static_cast<sharpen::MicroRpcVariableType>(this->stack_->Top().Header().type_));
        return;
    case Step::WaitSize:
        if (this->begin_ != this->end_)
        {
            ++this->begin_;
            this->stack_->Top().RawData().PushBack(data);
            return;
        }
#ifdef SHARPEN_IS_BIG_ENDIAN
        sharpen::ConvertEndian(this->stack_.Top().RawData().Data() + 1, this->end_);
#endif
        this->step_ = Step::WaitData;
        this->begin_ = 0;
        {
            sharpen::Size size{0};
            std::memcpy(&size, this->stack_->Top().RawData().Data() + 1, this->end_);
            this->end_ = sharpen::GetMicroRpcTypeSize(static_cast<sharpen::MicroRpcVariableType>(this->stack_->Top().Header().type_)) * size;
        }
    case Step::WaitData:
        if (this->begin_ != this->end_)
        {
            ++this->begin_;
            this->stack_->Top().RawData().PushBack(data);
            return;
        }
#ifdef SHARPEN_IS_BIG_ENDIAN
        sharpen::ConvertEndian(this->stack_.Top().RawData().Data() - this->end_, this->end_);
#endif
        this->step_ = Step::Completed;
    case Step::Completed:
    case Step::Error:
        return;
    }
}

#ifdef SHARPEN_COMPILER_GCC
#pragma GCC diagnostic push
#elif (defined SHARPEN_COMPILER_MSVC)
#pragma warning(pop)
#else
#endif

sharpen::Size sharpen::MicroRpcDecoder::Decode(const char *data, sharpen::Size size)
{
    assert(this->stack_);
    const char *begin = data;
    const char *end = data + size;
    while (begin != end)
    {
        this->step_ = Step::WaitMetadata;
        while (begin != end)
        {
            this->RunStateMachine(*begin);
            if(this->step_ == Step::Completed)
            {
                break;
            }
            ++begin;
        }
        this->begin_ = 0;
        this->end_ = 0;
        if(this->stack_->Top().Header().end_)
        {
            break;
        }
    }
    //set completed
    this->completed_ = true;
    this->stack_->Reverse();
    return begin - data;
}