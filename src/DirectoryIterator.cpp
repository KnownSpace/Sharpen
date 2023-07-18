#include <sharpen/DirectoryIterator.hpp>

#include <sharpen/Directory.hpp>
#include <cassert>

sharpen::DirectoryIterator::DirectoryIterator(sharpen::Directory *dir) noexcept
    : dir_(dir)
    , dentry_() {
    if (this->dir_) {
        this->Next();
    }
}

sharpen::DirectoryIterator::DirectoryIterator(Self &&other) noexcept
    : dir_(other.dir_)
    , dentry_(std::move(other.dentry_)) {
    other.dir_ = nullptr;
}

sharpen::DirectoryIterator &sharpen::DirectoryIterator::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->dir_ = other.dir_;
        this->dentry_ = std::move(other.dentry_);
        other.dir_ = nullptr;
    }
    return *this;
}

void sharpen::DirectoryIterator::Next() {
    assert(this->dir_ != nullptr);
    this->dentry_ = this->dir_->GetNextEntry();
    if (!this->dentry_.Valid()) {
        this->dir_ = nullptr;
    }
}

void sharpen::DirectoryIterator::operator++() {
    this->Next();
}

void sharpen::DirectoryIterator::operator++(int) {
    this->Next();
}

bool sharpen::DirectoryIterator::operator==(const Self &other) const noexcept {
    if (!this->dir_) {
        return !other.dir_;
    } else if (!other.dir_) {
        return !this->dir_;
    }
    return this->dir_ == other.dir_ && this->dentry_ == other.dentry_;
}

bool sharpen::DirectoryIterator::operator!=(const Self &other) const noexcept {
    return !(*this == other);
}

const sharpen::Dentry &sharpen::DirectoryIterator::operator*() const noexcept {
    assert(this->dir_ != nullptr);
    return this->dentry_;
}

const sharpen::Dentry *sharpen::DirectoryIterator::operator->() const noexcept {
    assert(this->dir_ != nullptr);
    return &this->dentry_;
}