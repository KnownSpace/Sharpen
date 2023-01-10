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

sharpen::RaftVoteForRequest sharpen::RaftMailExtractor::NviExtractVoteRequest(const sharpen::Mail &mail) const
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::VoteRequest);
    if(form.GetType() != sharpen::RaftMailType::VoteRequest)
    {
        throw std::invalid_argument{"unexpected mail"};
    }
    if(!form.CheckMagic())
    {
        throw sharpen::CorruptedDataError{"corrupted vote request mail"};
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        throw sharpen::CorruptedDataError{"corrupted vote request mail"};
    }
    sharpen::RaftVoteForRequest request;
    sharpen::BinarySerializator::LoadFrom(request,content);
    return request;
}


sharpen::RaftVoteForResponse sharpen::RaftMailExtractor::NviExtractVoteResponse(const sharpen::Mail &mail) const
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == sharpen::RaftMailType::VoteResponse);
    if(form.GetType() != sharpen::RaftMailType::VoteResponse)
    {
        throw std::invalid_argument{"unexpected mail"};
    }
    if(!form.CheckMagic())
    {
        throw sharpen::CorruptedDataError{"corrupted vote response mail"};
    }
    if(!form.CheckContent(content.GetSlice()))
    {
        throw sharpen::CorruptedDataError{"corrupted vote response mail"};
    }
    sharpen::RaftVoteForResponse response;
    sharpen::BinarySerializator::LoadFrom(response,content);
    return response;
}