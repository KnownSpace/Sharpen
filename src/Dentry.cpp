#include <sharpen/Dentry.hpp>

sharpen::Dentry::Dentry(std::string path) noexcept
    : Self{sharpen::DentryType::File, std::move(path)}
{
}

sharpen::Dentry::Dentry(sharpen::DentryType type, std::string path) noexcept
    : type_(type)
    , path_(std::move(path))
{
}

sharpen::Dentry::Dentry(const Self &other)
    : type_(other.type_)
    , path_(other.path_)
{
}

sharpen::Dentry::Dentry(Self &&other) noexcept
    :type_(other.type_)
    ,path_(std::move(other.path_))
{
    other.type_ = sharpen::DentryType::File;
}

sharpen::Dentry &sharpen::Dentry::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->type_ = other.type_;
        this->path_ = std::move(other.path_);
        other.type_ = sharpen::DentryType::File;
    }
    return *this;
}