#include <sharpen/CommonId.hpp>

#include <cstring>

sharpen::CommonId::CommonId() noexcept
    : data_() {
    this->Zero();
}

sharpen::CommonId::CommonId(const Self &other) noexcept
    : data_() {
    std::memcpy(this->data_, other.data_, CommonIdSize);
}

sharpen::CommonId::CommonId(Self &&other) noexcept 
    : data_(){
    std::memcpy(this->data_, other.data_, CommonIdSize);
    other.Zero();
}

sharpen::CommonId &sharpen::CommonId::operator=(const Self &other) noexcept {
    if (this != std::addressof(other)) {
        std::memcpy(this->data_, other.data_, CommonIdSize);
    }
    return *this;
}

sharpen::CommonId &sharpen::CommonId::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        std::memcpy(this->data_, other.data_, CommonIdSize);
        other.Zero();
    }
    return *this;
}

char *sharpen::CommonId::Data() noexcept {
    return this->data_;
}

const char *sharpen::CommonId::Data() const noexcept {
    return this->data_;
}

void sharpen::CommonId::Zero() noexcept {
    std::memset(this->data_,0,CommonIdSize);
}