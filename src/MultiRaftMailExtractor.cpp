#include <sharpen/MultiRaftMailExtractor.hpp>

#include <sharpen/MultiRaftForm.hpp>
#include <sharpen/GenericMail.hpp>

sharpen::MultiRaftMailExtractor::MultiRaftMailExtractor(std::uint32_t magic,std::uint32_t raftNumber) noexcept
    :magic_(magic)
    ,raftNumber_(raftNumber)
{}

sharpen::MultiRaftMailExtractor::MultiRaftMailExtractor(Self &&other) noexcept
    :magic_(other.magic_)
    ,raftNumber_(other.raftNumber_)
{
    other.magic_ = 0;
    other.raftNumber_ = 0;
}

bool sharpen::MultiRaftMailExtractor::NviIsRaftMail(const sharpen::Mail &mail) const noexcept
{
    if(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader))
    {
        const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
        if(header.GetMagic() == this->magic_)
        {
            const sharpen::MultiRaftForm &form{header.Form<sharpen::MultiRaftForm>()};
            return form.CheckMagic() && form.GetType() != sharpen::RaftMailType::Unknown && form.GetRaftNumber() == this->raftNumber_;
        }
    }
    return false;
}

sharpen::RaftMailType sharpen::MultiRaftMailExtractor::NviGetMailType(const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::MultiRaftForm &form{header.Form<sharpen::MultiRaftForm>()};
    assert(form.GetRaftNumber() == this->raftNumber_);
    return form.GetType();
}

bool sharpen::MultiRaftMailExtractor::CheckMail(sharpen::RaftMailType expect,const sharpen::Mail &mail) const noexcept
{
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::MultiRaftForm &form{header.Form<sharpen::MultiRaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == expect);
    assert(form.GetRaftNumber() == this->raftNumber_);
    return form.GetType() == expect && form.CheckMagic() && form.CheckContent(content.GetSlice());
}

sharpen::Optional<sharpen::RaftVoteForRequest> sharpen::MultiRaftMailExtractor::NviExtractVoteRequest(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::VoteRequest,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftVoteForRequest request;
    try
    {
        request.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}


sharpen::Optional<sharpen::RaftVoteForResponse> sharpen::MultiRaftMailExtractor::NviExtractVoteResponse(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::VoteResponse,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftVoteForResponse response;
    try
    {
        response.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftHeartbeatRequest> sharpen::MultiRaftMailExtractor::NviExtractHeartbeatRequest(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::HeartbeatRequest,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftHeartbeatRequest request;
    try
    {
        request.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftHeartbeatResponse> sharpen::MultiRaftMailExtractor::NviExtractHeartbeatResponse(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::HeartbeatResponse,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftHeartbeatResponse response;
    try
    {
        response.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftPrevoteRequest> sharpen::MultiRaftMailExtractor::NviExtractPrevoteRequest(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::PrevoteRequest,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftPrevoteRequest request;
    try
    {
        request.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftPrevoteResponse> sharpen::MultiRaftMailExtractor::NviExtractPrevoteResponse(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::PrevoteResponse,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftPrevoteResponse response;
    try
    {
        response.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftSnapshotRequest> sharpen::MultiRaftMailExtractor::NviExtractSnapshotRequest(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::InstallSnapshotRequest,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftSnapshotRequest request;
    try
    {
        request.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftSnapshotResponse> sharpen::MultiRaftMailExtractor::NviExtractSnapshotResponse(const sharpen::Mail &mail) const noexcept
{
    if(!this->CheckMail(sharpen::RaftMailType::InstallSnapshotResponse,mail))
    {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftSnapshotResponse response;
    try
    {
        response.Unserialize().LoadFrom(mail.Content());
    }
    catch(const sharpen::CorruptedDataError &error)
    {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}