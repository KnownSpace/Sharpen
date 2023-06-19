#include <sharpen/TcpStreamOption.hpp>

sharpen::TcpStreamOption::TcpStreamOption() noexcept
    : reuseAddr_(false) {
}

sharpen::TcpStreamOption::TcpStreamOption(Self &&other) noexcept
    : reuseAddr_(other.reuseAddr_) {
    other.reuseAddr_ = false;
}

sharpen::TcpStreamOption &sharpen::TcpStreamOption::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->reuseAddr_ = other.reuseAddr_;
        other.reuseAddr_ = false;
    }
    return *this;
}

bool sharpen::TcpStreamOption::EnableReuseAddress() const noexcept {
    return this->reuseAddr_;
}

void sharpen::TcpStreamOption::SetReuseAddress(bool reuse) noexcept {
    this->reuseAddr_ = reuse;
}