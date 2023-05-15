#include <sharpen/GenericMailPaser.hpp>


sharpen::GenericMailPaser::GenericMailPaser(std::uint32_t magic) noexcept
    : magic_(magic)
    , parsedSize_(0)
    , header_()
    , content_()
    , completedMails_() {
    this->header_.ExtendTo(sizeof(sharpen::GenericMailHeader));
}

sharpen::GenericMailPaser::GenericMailPaser(const Self &other)
    : magic_(other.magic_)
    , parsedSize_(other.parsedSize_)
    , header_(other.header_)
    , content_(other.content_)
    , completedMails_(other.completedMails_) {
}

sharpen::GenericMailPaser::GenericMailPaser(Self &&other) noexcept
    : magic_(other.magic_)
    , parsedSize_(other.parsedSize_)
    , header_(std::move(other.header_))
    , content_(std::move(other.content_))
    , completedMails_(std::move(other.completedMails_)) {
    other.magic_ = 0;
    other.parsedSize_ = 0;
}

sharpen::GenericMailPaser &sharpen::GenericMailPaser::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->magic_ = other.magic_;
        this->parsedSize_ = other.parsedSize_;
        this->header_ = std::move(other.header_);
        this->content_ = std::move(other.content_);
        this->completedMails_ = std::move(other.completedMails_);
        other.magic_ = 0;
        other.parsedSize_ = 0;
    }
    return *this;
}

bool sharpen::GenericMailPaser::Completed() const noexcept {
    return !this->completedMails_.empty();
}

void sharpen::GenericMailPaser::NviParse(sharpen::ByteSlice slice) {
    while (!slice.Empty()) {
        const char *data{slice.Data()};
        std::size_t offset{0};
        std::size_t copyParsed{this->parsedSize_};
        if (this->parsedSize_ < sizeof(sharpen::GenericMailHeader)) {
            std::size_t size{(std::min)(slice.GetSize(),
                                        sizeof(sharpen::GenericMailHeader) - this->parsedSize_)};
            std::memcpy(this->header_.Data() + this->parsedSize_, data, size);
            offset = size;
            this->parsedSize_ += size;
            if (this->parsedSize_ < sizeof(sharpen::GenericMailHeader)) {
                return;
            }
            std::uint32_t magic{this->header_.As<sharpen::GenericMailHeader>().GetMagic()};
            if (magic != this->magic_) {
                throw sharpen::MailParseError{"Unexcepted magic number"};
            }
        }
        std::size_t contentSize{this->header_.As<sharpen::GenericMailHeader>().GetContentSize()};
        if (!contentSize || slice.GetSize() == offset) {
            return;
        }
        if (this->content_.Empty()) {
            this->content_.ExtendTo(contentSize);
        }
        std::size_t size{(std::min)(contentSize, slice.GetSize() - offset)};
        std::size_t contentOffset{this->parsedSize_ - sizeof(sharpen::GenericMailHeader)};
        std::memcpy(this->content_.Data() + contentOffset, data + offset, size);
        this->parsedSize_ += size;
        if (this->parsedSize_ == sizeof(sharpen::GenericMailHeader) + contentSize) {
            sharpen::ByteBuffer header{sizeof(sharpen::GenericMailHeader)};
            sharpen::ByteBuffer content;
            std::swap(header,this->header_);
            std::swap(content,this->content_);
            this->completedMails_.emplace_back(std::move(header),std::move(content));
            slice = slice.Sub(this->parsedSize_ - copyParsed);
            this->parsedSize_ = 0;
        }
    }
}

sharpen::Mail sharpen::GenericMailPaser::NviPopCompletedMail() noexcept {
    sharpen::Mail mail{this->completedMails_.front()};
    this->completedMails_.pop_front();
    return mail;
}