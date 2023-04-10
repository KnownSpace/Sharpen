#include <sharpen/RaftMailBuilder.hpp>

#include <sharpen/GenericMail.hpp>
#include <sharpen/IntOps.hpp>
#include <sharpen/BufferWriter.hpp>

sharpen::RaftMailBuilder::RaftMailBuilder(std::uint32_t magic) noexcept
    :magic_(magic)
{}

sharpen::RaftMailBuilder::RaftMailBuilder(Self &&other) noexcept
    :magic_(other.magic_)
{
    other.magic_ = 0;
}

sharpen::RaftMailBuilder &sharpen::RaftMailBuilder::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        other.magic_ = 0;
    }
    return *this;
}

sharpen::RaftForm sharpen::RaftMailBuilder::BuildForm(sharpen::RaftMailType type,const sharpen::ByteBuffer &content) const noexcept
{
    assert(type != sharpen::RaftMailType::MaxValue);
    assert(type != sharpen::RaftMailType::Unknown);
    sharpen::RaftForm form;
    form.SetType(type);
    form.SetChecksum(content.GetSlice());
    return form;
}

sharpen::Mail sharpen::RaftMailBuilder::BuildMail(sharpen::RaftMailType type,sharpen::ByteBuffer content) const noexcept
{
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = this->BuildForm(type,content);
    mail.SetContent(std::move(content));
    return mail.ReleaseMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildVoteRequest(const sharpen::RaftVoteForRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::VoteRequest,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildVoteResponse(const sharpen::RaftVoteForResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::VoteResponse,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildHeartbeatRequest(const sharpen::RaftHeartbeatRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::HeartbeatRequest,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildHeartbeatResponse(const sharpen::RaftHeartbeatResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::HeartbeatResponse,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildPrevoteRequest(const sharpen::RaftPrevoteRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::PrevoteRequest,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildPrevoteResponse(const sharpen::RaftPrevoteResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::PrevoteResponse,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildSnapshotRequest(const sharpen::RaftSnapshotRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::InstallSnapshotRequest,std::move(content));
}

sharpen::Mail sharpen::RaftMailBuilder::BuildSnapshotResponse(const sharpen::RaftSnapshotResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::InstallSnapshotResponse,std::move(content));
}