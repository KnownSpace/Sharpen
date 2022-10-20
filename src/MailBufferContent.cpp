#include <sharpen/MailBufferContent.hpp>

sharpen::MailBufferContent::MailBufferContent(sharpen::ByteBuffer content)
    :content_(std::move(content))
{}

char *sharpen::MailBufferContent::DoContent() noexcept
{
    return this->content_.Data();
}

const char *sharpen::MailBufferContent::DoContent() const noexcept
{
    return this->content_.Data();
}

std::size_t sharpen::MailBufferContent::DoGetSize() const noexcept
{
    return this->content_.GetSize();
}

void sharpen::MailBufferContent::DoResize(std::size_t newSize)
{
    return this->content_.ExtendTo(newSize);
}