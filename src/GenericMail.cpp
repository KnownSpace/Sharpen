#include <sharpen/GenericMail.hpp>

#include <sharpen/IntOps.hpp>

sharpen::GenericMail::GenericMail() noexcept
    :Self(0)
{}

sharpen::GenericMail::GenericMail(std::uint32_t magic) noexcept
    :Base()
{
    this->Header().ExtendTo(sizeof(sharpen::GenericMailHeader));
    this->Header().As<sharpen::GenericMailHeader>().SetMagic(magic);
}

sharpen::GenericMailHeader &sharpen::GenericMail::GenericHeader() noexcept
{
    assert(this->Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    return this->Header().As<sharpen::GenericMailHeader>();
}

const sharpen::GenericMailHeader &sharpen::GenericMail::GenericHeader() const noexcept
{
    assert(this->Header().GetSize() == sizeof(sharpen::GenericMailHeader));
    return this->Header().As<sharpen::GenericMailHeader>();
}

std::uint32_t sharpen::GenericMail::GetMagic() const noexcept
{
    return this->GenericHeader().GetMagic();
}

void sharpen::GenericMail::SetMagic(std::uint32_t magic) noexcept
{
    this->GenericHeader().SetMagic(magic);
}

void sharpen::GenericMail::SetContent(sharpen::ByteBuffer content)
{
    //overflow check
    std::uint32_t contentSize{sharpen::IntCast<std::uint32_t>(content.GetSize())};
    this->GenericHeader().SetContentSize(contentSize);
    Base::Content() = std::move(content);
}