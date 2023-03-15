#include <sharpen/RaftMailExtractor.hpp>

#include <sharpen/GenericMail.hpp>
#include <sharpen/RaftForm.hpp>

sharpen::RaftMailExtractor::RaftMailExtractor(std::uint32_t magic) noexcept
    :magic_(magic)
{}

sharpen::RaftMailExtractor::RaftMailExtractor(Self &&other) noexcept
    :magic_(other.magic_)
{
    other.magic_ = 0;
}

sharpen::RaftMailExtractor &sharpen::RaftMailExtractor::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
    }
    return *this;
}

bool sharpen::RaftMailExtractor::NviIsRaftMail(const sharpen::Mail &mail) const noexcept
{
    if(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader))
    {
        const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
        if(header.GetMagic() == this->magic_)
        {
            const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
            return form.CheckMagic() && form.GetType() != sharpen::RaftMailType::Unknown;
        }
    }
    return false;
}

sharpen::RaftMailType sharpen::RaftMailExtractor::NviGetMailType(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    return form.GetType();
}

sharpen::Optional<sharpen::RaftVoteForRequest> sharpen::RaftMailExtractor::NviExtractVoteRequest(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::VoteRequest);
    if(form.GetType() != sharpen::RaftMailType::VoteRequest)
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckMagic())
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftVoteForRequest request;
    try
    {
        request.Unserialize().LoadFrom(content);
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}


sharpen::Optional<sharpen::RaftVoteForResponse> sharpen::RaftMailExtractor::NviExtractVoteResponse(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::VoteResponse);
    if(form.GetType() != sharpen::RaftMailType::VoteResponse)
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckMagic())
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftVoteForResponse response;
    try
    {
        response.Unserialize().LoadFrom(content);
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftHeartbeatRequest> sharpen::RaftMailExtractor::NviExtractHeartbeatRequest(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::HeartbeatRequest);
    if(form.GetType() != sharpen::RaftMailType::HeartbeatRequest)
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckMagic())
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftHeartbeatRequest request;
    try
    {
        request.Unserialize().LoadFrom(content);
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftHeartbeatResponse> sharpen::RaftMailExtractor::NviExtractHeartbeatResponse(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::HeartbeatResponse);
    if(form.GetType() != sharpen::RaftMailType::HeartbeatResponse)
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckMagic())
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftHeartbeatResponse response;
    try
    {
        response.Unserialize().LoadFrom(content);
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftPrevoteRequest> sharpen::RaftMailExtractor::NviExtractPrevoteRequest(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::PrevoteRequest);
    if(form.GetType() != sharpen::RaftMailType::PrevoteRequest)
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckMagic())
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftPrevoteRequest request;
    try
    {
        request.Unserialize().LoadFrom(content);
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftPrevoteResponse> sharpen::RaftMailExtractor::NviExtractPrevoteResponse(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::PrevoteResponse);
    if(form.GetType() != sharpen::RaftMailType::PrevoteResponse)
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckMagic())
    {
        return sharpen::EmptyOpt;
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftPrevoteResponse response;
    try
    {
        response.Unserialize().LoadFrom(content);
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}