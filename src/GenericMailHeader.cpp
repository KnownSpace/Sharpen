#include <sharpen/GenericMailHeader.hpp>

#include <cstring>

#include <sharpen/ByteOrder.hpp>

sharpen::GenericMailHeader::GenericMailHeader(std::uint32_t magic) noexcept
    :Self(magic,0)
{}

sharpen::GenericMailHeader::GenericMailHeader(std::uint32_t magic,std::uint32_t size) noexcept
    :magic_(0)
    ,contentSize_(0)
    ,form_()
{
    std::memset(this->form_,0,sizeof(this->form_));
    this->SetMagic(magic);
    this->SetContentSize(size);
}

sharpen::GenericMailHeader::GenericMailHeader(const Self &other) noexcept
    :magic_(other.magic_)
    ,contentSize_(other.contentSize_)
    ,form_()
{
    std::memcpy(this->form_,other.form_,sizeof(this->form_));
}

sharpen::GenericMailHeader::GenericMailHeader(Self &&other) noexcept
    :magic_(other.magic_)
    ,contentSize_(other.contentSize_)
    ,form_()
{
    std::memcpy(this->form_,other.form_,sizeof(this->form_));
    other.magic_ = 0;
    other.contentSize_ = 0;
}

sharpen::GenericMailHeader &sharpen::GenericMailHeader::operator=(const Self &other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        this->contentSize_ = other.contentSize_;
        std::memcpy(this->form_,other.form_,sizeof(this->form_));
    }
    return *this;
}

sharpen::GenericMailHeader &sharpen::GenericMailHeader::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        this->contentSize_ = other.contentSize_;
        std::memcpy(this->form_,other.form_,sizeof(this->form_));
        other.contentSize_ = 0;
        other.magic_ = 0;
    }
    return *this;
}

std::uint32_t sharpen::GenericMailHeader::GetContentSize() const noexcept
{
    std::uint32_t size{this->contentSize_};
#ifndef SHARPEN_IS_LIL_ENDIAN
    sharpen::ConvertEndian(size);
#endif
    return size;
}

void sharpen::GenericMailHeader::SetContentSize(std::uint32_t contentSize) noexcept
{
#ifndef SHARPEN_IS_LIL_ENDIAN
    sharpen::ConvertEndian(contentSize);
#endif
    this->contentSize_ = contentSize;
}

std::uint32_t sharpen::GenericMailHeader::GetMagic() const noexcept
{
    std::uint32_t magic{this->magic_};
#ifndef SHARPEN_IS_LIL_ENDIAN
    sharpen::ConvertEndian(magic);
#endif
    return magic;
}

void sharpen::GenericMailHeader::SetMagic(std::uint32_t magic) noexcept
{

#ifndef SHARPEN_IS_LIL_ENDIAN
    sharpen::ConvertEndian(magic);
#endif
    this->magic_ = magic;
}