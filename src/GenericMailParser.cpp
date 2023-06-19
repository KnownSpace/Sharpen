#include <sharpen/GenericMailParser.hpp>


sharpen::GenericMailParser::GenericMailParser(std::uint32_t magic) noexcept
    : magic_(magic)
    , maxContentSize_(Self::defaultMaxContentSize)
    , parsedSize_(0)
    , header_()
    , content_()
    , completedMails_() {
    this->header_.ExtendTo(sizeof(sharpen::GenericMailHeader));
}

sharpen::GenericMailParser::GenericMailParser(const Self &other)
    : magic_(other.magic_)
    , maxContentSize_(other.maxContentSize_)
    , parsedSize_(other.parsedSize_)
    , header_(other.header_)
    , content_(other.content_)
    , completedMails_(other.completedMails_) {
}

sharpen::GenericMailParser::GenericMailParser(Self &&other) noexcept
    : magic_(other.magic_)
    , maxContentSize_(other.maxContentSize_)
    , parsedSize_(other.parsedSize_)
    , header_(std::move(other.header_))
    , content_(std::move(other.content_))
    , completedMails_(std::move(other.completedMails_)) {
    other.magic_ = 0;
    other.parsedSize_ = 0;
    other.maxContentSize_ = Self::defaultMaxContentSize;
}

sharpen::GenericMailParser &sharpen::GenericMailParser::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->magic_ = other.magic_;
        this->maxContentSize_ = other.maxContentSize_;
        this->parsedSize_ = other.parsedSize_;
        this->header_ = std::move(other.header_);
        this->content_ = std::move(other.content_);
        this->completedMails_ = std::move(other.completedMails_);
        other.magic_ = 0;
        other.parsedSize_ = 0;
        other.maxContentSize_ = Self::defaultMaxContentSize;
    }
    return *this;
}

bool sharpen::GenericMailParser::Completed() const noexcept {
    return !this->completedMails_.empty();
}

sharpen::GenericMailParser::ParseStatus sharpen::GenericMailParser::GetStatus() const noexcept {
    if (this->parsedSize_ >= sizeof(sharpen::GenericMailHeader)) {
        return ParseStatus::Content;
    }
    return ParseStatus::Header;
}

void sharpen::GenericMailParser::NviParse(sharpen::ByteSlice slice) {
    for (auto begin = slice.Begin(), end = slice.End(); begin != end;) {
        switch (this->GetStatus()) {
        case ParseStatus::Header: {
            assert(this->header_.GetSize() == sizeof(sharpen::GenericMailHeader));
            std::size_t offset{this->parsedSize_};
            std::size_t remainSize{static_cast<std::size_t>(end - begin)};
            remainSize = (std::min)(remainSize, sizeof(sharpen::GenericMailHeader) - offset);
            std::memcpy(this->header_.Data() + offset, begin.GetPointer(), remainSize);
            this->parsedSize_ += remainSize;
            begin += remainSize;
        } break;
        case ParseStatus::Content: {
            assert(this->header_.GetSize() == sizeof(sharpen::GenericMailHeader));
            assert(this->parsedSize_ >= this->header_.GetSize());
            const sharpen::GenericMailHeader *header{
                &this->header_.As<sharpen::GenericMailHeader>()};
            std::uint32_t contentSize{header->GetContentSize()};
            if (contentSize > this->maxContentSize_) {
                throw sharpen::MailParseError{"content too long"};
            }
            if (this->content_.Empty()) {
                this->content_.ExtendTo(contentSize);
            }
            std::size_t offset{this->parsedSize_ - sizeof(sharpen::GenericMailHeader)};
            std::size_t remainSize{contentSize - offset};
            remainSize = (std::min)(remainSize, static_cast<std::size_t>(end - begin));
            std::memcpy(this->content_.Data() + offset, begin.GetPointer(), remainSize);
            this->parsedSize_ += remainSize;
            if (this->parsedSize_ == sizeof(sharpen::GenericMailHeader) + contentSize) {
                sharpen::ByteBuffer headerBuf{sizeof(sharpen::GenericMailHeader)};
                sharpen::ByteBuffer contentBuf;
                std::swap(headerBuf, this->header_);
                std::swap(contentBuf, this->content_);
                this->completedMails_.emplace_back(std::move(headerBuf), std::move(contentBuf));
                this->parsedSize_ = 0;
            }
            begin += remainSize;
        } break;
        }
    }
}

sharpen::Mail sharpen::GenericMailParser::NviPopCompletedMail() noexcept {
    sharpen::Mail mail{this->completedMails_.front()};
    this->completedMails_.pop_front();
    return mail;
}

void sharpen::GenericMailParser::PrepareMaxContentSize(std::uint32_t maxContentSize) noexcept {
    if (maxContentSize < Self::minMaxContentSize) {
        maxContentSize = Self::minMaxContentSize;
    }
    this->maxContentSize_ = maxContentSize;
}