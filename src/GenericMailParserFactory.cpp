#include <sharpen/GenericMailParserFactory.hpp>

#include <sharpen/GenericMailPaser.hpp>

sharpen::GenericMailParserFactory::GenericMailParserFactory(std::uint32_t magic) noexcept
    : magic_(magic)
{
}

sharpen::GenericMailParserFactory::GenericMailParserFactory(Self &&other) noexcept
    : magic_(other.magic_)
{
    other.magic_ = 0;
}

sharpen::GenericMailParserFactory &sharpen::GenericMailParserFactory::operator=(
    Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        other.magic_ = 0;
    }
    return *this;
}

std::unique_ptr<sharpen::IMailParser> sharpen::GenericMailParserFactory::Produce()
{
    std::unique_ptr<sharpen::IMailParser> parser{new (std::nothrow)
                                                     sharpen::GenericMailPaser{this->magic_}};
    if (!parser)
    {
        throw std::bad_alloc{};
    }
    return parser;
}