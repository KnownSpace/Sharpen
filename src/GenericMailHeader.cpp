#include <sharpen/GenericMailHeader.hpp>

#include <cstring>

sharpen::GenericMailHeader::GenericMailHeader(std::uint32_t magic) noexcept
    :Self(magic,0)
{}

sharpen::GenericMailHeader::GenericMailHeader(std::uint32_t magic,std::uint32_t size) noexcept
    :magic_(magic)
    ,contentSize_(size)
    ,form_()
{
    std::memset(this->form_,0,sizeof(this->form_));
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

