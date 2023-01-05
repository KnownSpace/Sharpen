#include <sharpen/GenericMailPaser.hpp>

sharpen::GenericMailPaser::GenericMailPaser(std::uint32_t magic) noexcept
    :magic_(magic)
    ,parsedSize_(0)
    ,header_()
    ,content_()
{
    this->header_.ExtendTo(sizeof(sharpen::GenericMailHeader));
}

sharpen::GenericMailPaser::GenericMailPaser(const Self &other)
    :magic_(other.magic_)
    ,parsedSize_(other.parsedSize_)
    ,header_(other.header_)
    ,content_(other.content_)
{}

sharpen::GenericMailPaser::GenericMailPaser(Self &&other) noexcept
    :magic_(other.magic_)
    ,parsedSize_(other.parsedSize_)
    ,header_(std::move(other.header_))
    ,content_(std::move(other.content_))
{
    other.magic_ = 0;
    other.parsedSize_ = 0;
}

sharpen::GenericMailPaser &sharpen::GenericMailPaser::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        this->parsedSize_ = other.parsedSize_;
        this->header_ = std::move(other.header_);
        this->content_ = std::move(other.content_);
        other.magic_ = 0;
        other.parsedSize_ = 0;
    }
    return *this;
}

bool sharpen::GenericMailPaser::Completed() const noexcept
{
    if(this->parsedSize_ < sizeof(sharpen::GenericMailHeader))
    {
        return false;
    }
    return this->parsedSize_ == sizeof(sharpen::GenericMailHeader) + this->header_.As<sharpen::GenericMailHeader>().GetContentSize();
}

void sharpen::GenericMailPaser::NviParse(sharpen::ByteSlice slice)
{
    const char *data{slice.Data()};
    std::size_t offset{0};
    if(this->parsedSize_ < sizeof(sharpen::GenericMailHeader))
    {
        std::size_t size{(std::min)(slice.GetSize(),sizeof(sharpen::GenericMailHeader) - this->parsedSize_)};
        std::memcpy(this->header_.Data() + this->parsedSize_,data,size);
        offset = size;
        this->parsedSize_ += size;
        if(this->parsedSize_ < sizeof(sharpen::GenericMailHeader))
        {
            return;
        }
        std::uint32_t magic{this->header_.As<sharpen::GenericMailHeader>().GetMagic()};
        if(magic != this->magic_)
        {
            throw sharpen::MailParseError{"Unexcepted magic number"};
        }
    }
    std::size_t contentSize{this->header_.As<sharpen::GenericMailHeader>().GetContentSize()};
    if(!contentSize || slice.GetSize() == offset)
    {
        return;
    }
    if(this->content_.Empty())
    {
        this->content_.ExtendTo(contentSize);
    }
    std::size_t size{(std::min)(contentSize,slice.GetSize() - offset)};
    std::size_t contentOffset{this->parsedSize_ - sizeof(sharpen::GenericMailHeader)};
    std::memcpy(this->content_.Data() + contentOffset,data + offset,size);
    this->parsedSize_ += size;
}

sharpen::Mail sharpen::GenericMailPaser::NviPopCompletedMail() noexcept
{
    sharpen::ByteBuffer header;
    sharpen::ByteBuffer content;
    header.ExtendTo(sizeof(sharpen::GenericMailHeader));
    std::swap(header,this->header_);
    std::swap(content,this->content_);
    sharpen::Mail mail{std::move(header),std::move(content)};
    return mail;
}