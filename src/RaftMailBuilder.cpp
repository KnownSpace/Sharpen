#include <sharpen/RaftMailBuilder.hpp>

#include <sharpen/GenericMail.hpp>
#include <sharpen/RaftForm.hpp>
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
    }
    return *this;
}

sharpen::Mail sharpen::RaftMailBuilder::BuildVoteRequest(const sharpen::RaftVoteForRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::VoteRequest};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildVoteResponse(const sharpen::RaftVoteForResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::VoteResponse};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildHeartbeatRequest(const sharpen::RaftHeartbeatRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::HeartbeatRequest};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildHeartbeatResponse(const sharpen::RaftHeartbeatResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::HeartbeatResponse};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildPrevoteRequest(const sharpen::RaftPrevoteRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::PrevoteRequest};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildPrevoteResponse(const sharpen::RaftPrevoteResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::PrevoteResponse};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildSnapshotRequest(const sharpen::RaftSnapshotRequest &request) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(request.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::InstallSnapshotRequest};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(request);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}

sharpen::Mail sharpen::RaftMailBuilder::BuildSnapshotResponse(const sharpen::RaftSnapshotResponse &response) const
{
    std::uint32_t size{sharpen::IntCast<std::uint32_t>(response.ComputeSize())};
    sharpen::RaftForm form{sharpen::RaftMailType::InstallSnapshotRequest};
    sharpen::ByteBuffer content{size};
    sharpen::BufferWriter writer{content};
    writer.Write(response);
    form.SetChecksum(content.GetSlice());
    sharpen::GenericMail mail{this->magic_};
    mail.Form<sharpen::RaftForm>() = form;
    mail.SetContent(std::move(content));
    return mail.AsMail();
}