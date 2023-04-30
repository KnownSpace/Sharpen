#include <sharpen/MultiRaftMailBuilder.hpp>

#include <sharpen/BufferWriter.hpp>
#include <sharpen/GenericMail.hpp>

sharpen::MultiRaftMailBuilder::MultiRaftMailBuilder(std::uint32_t magic,
                                                    std::uint32_t raftNumber) noexcept
    : magic_(magic)
    , raftNumber_(raftNumber)
{
}

sharpen::MultiRaftMailBuilder::MultiRaftMailBuilder(Self &&other) noexcept
    : magic_(other.magic_)
    , raftNumber_(other.raftNumber_)
{
    other.magic_ = 0;
    other.raftNumber_ = 0;
}

sharpen::MultiRaftMailBuilder &sharpen::MultiRaftMailBuilder::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        this->raftNumber_ = other.raftNumber_;
        other.magic_ = 0;
        other.raftNumber_ = 0;
    }
    return *this;
}

sharpen::MultiRaftForm sharpen::MultiRaftMailBuilder::BuildForm(
    sharpen::RaftMailType type, const sharpen::ByteBuffer &content) const noexcept
{
    assert(type != sharpen::RaftMailType::MaxValue);
    assert(type != sharpen::RaftMailType::Unknown);
    sharpen::MultiRaftForm form;
    form.SetRaftNumber(this->raftNumber_);
    form.SetType(type);
    form.SetChecksum(content.GetSlice());
    return form;
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildMail(sharpen::RaftMailType type,
                                                       sharpen::ByteBuffer content) const noexcept
{
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::MultiRaftForm>() = this->BuildForm(type, content);
    mail.SetContent(std::move(content));
    return mail.ReleaseMail();
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildVoteRequest(
    const sharpen::RaftVoteForRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::VoteRequest, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildVoteResponse(
    const sharpen::RaftVoteForResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::VoteResponse, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildPrevoteRequest(
    const sharpen::RaftPrevoteRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::PrevoteRequest, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildPrevoteResponse(
    const sharpen::RaftPrevoteResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::PrevoteResponse, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildHeartbeatRequest(
    const sharpen::RaftHeartbeatRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::HeartbeatRequest, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildHeartbeatResponse(
    const sharpen::RaftHeartbeatResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::HeartbeatResponse, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildSnapshotRequest(
    const sharpen::RaftSnapshotRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    return this->BuildMail(sharpen::RaftMailType::InstallSnapshotRequest, std::move(content));
}

sharpen::Mail sharpen::MultiRaftMailBuilder::BuildSnapshotResponse(
    const sharpen::RaftSnapshotResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    return this->BuildMail(sharpen::RaftMailType::InstallSnapshotResponse, std::move(content));
}