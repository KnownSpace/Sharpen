#include <sharpen/RaftMailExtractor.hpp>

#include <sharpen/GenericMail.hpp>
#include <sharpen/RaftForm.hpp>

sharpen::RaftMailExtractor::RaftMailExtractor(std::uint32_t magic) noexcept
    : magic_(magic) {
}

sharpen::RaftMailExtractor::RaftMailExtractor(Self &&other) noexcept
    : magic_(other.magic_) {
    other.magic_ = 0;
}

sharpen::RaftMailExtractor &sharpen::RaftMailExtractor::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->magic_ = other.magic_;
        other.magic_ = 0;
    }
    return *this;
}

bool sharpen::RaftMailExtractor::NviIsRaftMail(const sharpen::Mail &mail) const noexcept {
    if (mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader)) {
        const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
        if (header.GetMagic() == this->magic_) {
            const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
            return form.CheckMagic() && form.GetType() != sharpen::RaftMailType::Unknown;
        }
    }
    return false;
}

sharpen::RaftMailType sharpen::RaftMailExtractor::NviGetMailType(
    const sharpen::Mail &mail) const noexcept {
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    return form.GetType();
}

bool sharpen::RaftMailExtractor::CheckMail(sharpen::RaftMailType expect,
                                           const sharpen::Mail &mail) const noexcept {
    assert(mail.Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    const sharpen::GenericMailHeader &header{mail.Header().As<sharpen::GenericMailHeader>()};
    assert(header.GetMagic() == this->magic_);
    const sharpen::RaftForm &form{header.Form<sharpen::RaftForm>()};
    const sharpen::ByteBuffer &content{mail.Content()};
    assert(form.GetType() == expect);
    return form.GetType() == expect && form.CheckMagic() && form.CheckContent(content.GetSlice());
}

sharpen::Optional<sharpen::RaftVoteForRequest> sharpen::RaftMailExtractor::NviExtractVoteRequest(
    const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::VoteRequest, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftVoteForRequest request;
    try {
        request.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}


sharpen::Optional<sharpen::RaftVoteForResponse> sharpen::RaftMailExtractor::NviExtractVoteResponse(
    const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::VoteResponse, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftVoteForResponse response;
    try {
        response.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftHeartbeatRequest>
sharpen::RaftMailExtractor::NviExtractHeartbeatRequest(const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::HeartbeatRequest, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftHeartbeatRequest request;
    try {
        request.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftHeartbeatResponse>
sharpen::RaftMailExtractor::NviExtractHeartbeatResponse(const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::HeartbeatResponse, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftHeartbeatResponse response;
    try {
        response.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftPrevoteRequest> sharpen::RaftMailExtractor::NviExtractPrevoteRequest(
    const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::PrevoteRequest, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftPrevoteRequest request;
    try {
        request.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftPrevoteResponse>
sharpen::RaftMailExtractor::NviExtractPrevoteResponse(const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::PrevoteResponse, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftPrevoteResponse response;
    try {
        response.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}

sharpen::Optional<sharpen::RaftSnapshotRequest>
sharpen::RaftMailExtractor::NviExtractSnapshotRequest(const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::InstallSnapshotRequest, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftSnapshotRequest request;
    try {
        request.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return request;
}

sharpen::Optional<sharpen::RaftSnapshotResponse>
sharpen::RaftMailExtractor::NviExtractSnapshotResponse(const sharpen::Mail &mail) const noexcept {
    if (!this->CheckMail(sharpen::RaftMailType::InstallSnapshotResponse, mail)) {
        return sharpen::EmptyOpt;
    }
    sharpen::RaftSnapshotResponse response;
    try {
        response.Unserialize().LoadFrom(mail.Content());
    } catch (const sharpen::CorruptedDataError &error) {
        (void)error;
        return sharpen::EmptyOpt;
    }
    return response;
}