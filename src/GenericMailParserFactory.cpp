#include <sharpen/GenericMailParserFactory.hpp>

#include <sharpen/GenericMailParser.hpp>

sharpen::GenericMailParserFactory::GenericMailParserFactory(std::uint32_t magic) noexcept
    : Self{magic, sharpen::GenericMailParser::defaultMaxContentSize} {
}

sharpen::GenericMailParserFactory::GenericMailParserFactory(std::uint32_t magic,
                                                            std::uint32_t maxContentSize) noexcept
    : magic_(magic)
    , maxContentSize_(maxContentSize) {
}

sharpen::GenericMailParserFactory::GenericMailParserFactory(Self &&other) noexcept
    : magic_(other.magic_)
    , maxContentSize_(other.maxContentSize_) {
    other.magic_ = 0;
    other.maxContentSize_ = sharpen::GenericMailParser::defaultMaxContentSize;
}

sharpen::GenericMailParserFactory &sharpen::GenericMailParserFactory::operator=(
    Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->magic_ = other.magic_;
        this->maxContentSize_ = other.maxContentSize_;
        other.magic_ = 0;
        other.maxContentSize_ = sharpen::GenericMailParser::defaultMaxContentSize;
    }
    return *this;
}

std::unique_ptr<sharpen::IMailParser> sharpen::GenericMailParserFactory::Produce() {
    std::unique_ptr<sharpen::GenericMailParser> parser{new (std::nothrow)
                                                     sharpen::GenericMailParser{this->magic_}};
    if (!parser) {
        throw std::bad_alloc{};
    }
    parser->PrepareMaxContentSize(this->maxContentSize_);
    return parser;
}