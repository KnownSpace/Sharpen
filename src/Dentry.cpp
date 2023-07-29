#include <sharpen/Dentry.hpp>

sharpen::Dentry::Dentry() noexcept
    : type_(sharpen::FileEntryType::File)
    , name_() {
}

sharpen::Dentry::Dentry(std::string name) noexcept
    : Self{sharpen::FileEntryType::File, std::move(name)} {
}

sharpen::Dentry::Dentry(sharpen::FileEntryType type, std::string name) noexcept
    : type_(type)
    , name_(std::move(name)) {
}

sharpen::Dentry::Dentry(const Self &other)
    : type_(other.type_)
    , name_(other.name_) {
}

sharpen::Dentry::Dentry(Self &&other) noexcept
    : type_(other.type_)
    , name_(std::move(other.name_)) {
    other.type_ = sharpen::FileEntryType::File;
}

sharpen::Dentry &sharpen::Dentry::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->type_ = other.type_;
        this->name_ = std::move(other.name_);
        other.type_ = sharpen::FileEntryType::File;
    }
    return *this;
}

bool sharpen::Dentry::Valid() const noexcept {
    return !this->name_.empty();
}

bool sharpen::Dentry::operator==(const Self &other) const noexcept {
    return this->name_ == other.name_ && this->type_ == other.type_;
}

bool sharpen::Dentry::operator!=(const Self &other) const noexcept {
    return !(*this == other);
}