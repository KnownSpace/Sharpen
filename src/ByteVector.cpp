#include <sharpen/ByteVector.hpp>

#include <cassert>

void *sharpen::ByteVector::Alloc(std::size_t size) noexcept {
    assert(size);
    return std::calloc(size, sizeof(char));
}

void sharpen::ByteVector::Free(void *p) noexcept {
    return std::free(p);
}

std::size_t sharpen::ByteVector::ComputeHeapSize(std::size_t size) noexcept {
    if (size % 2) {
        size += 1;
    }
    return size;
}

sharpen::ByteVector::ByteVector(std::size_t size)
    : size_(0)
    , rawVector_() {
    if (!this->InlineBuffer(size)) {
        std::size_t cap{this->ComputeHeapSize(size)};
        char *buf = reinterpret_cast<char *>(this->Alloc(cap));
        if (!buf) {
            throw std::bad_alloc();
        }
        this->rawVector_.external_.data_ = buf;
        this->rawVector_.external_.cap_ = cap;
    }
    this->size_ = size;
}

sharpen::ByteVector::ByteVector(const Self &other)
    : size_(0)
    , rawVector_() {
    if (!other.InlineBuffer()) {
        std::size_t cap{other.rawVector_.external_.cap_};
        char *buf{reinterpret_cast<char *>(this->Alloc(cap))};
        if (!buf) {
            throw std::bad_alloc();
        }
        std::memcpy(buf, other.rawVector_.external_.data_, other.size_);
        this->rawVector_.external_.data_ = buf;
        this->rawVector_.external_.cap_ = cap;
    } else if (other.size_) {
        std::memcpy(this->rawVector_.inline_, other.rawVector_.inline_, other.size_);
    }
    this->size_ = other.size_;
}

void sharpen::ByteVector::MoveFrom(Self &&other) noexcept {
    this->Clear();
    if (!other.InlineBuffer()) {
        this->rawVector_.external_.cap_ = other.rawVector_.external_.cap_;
        this->rawVector_.external_.data_ = other.rawVector_.external_.data_;
        other.rawVector_.external_.data_ = nullptr;
        other.rawVector_.external_.cap_ = 0;
    } else if (other.size_) {
        std::memcpy(this->rawVector_.inline_, other.rawVector_.inline_, other.size_);
    }
    std::swap(this->size_, other.size_);
}

sharpen::ByteVector::ByteVector(Self &&other) noexcept
    : size_(0)
    , rawVector_() {
    this->MoveFrom(std::move(other));
}

sharpen::ByteVector &sharpen::ByteVector::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->MoveFrom(std::move(other));
    }
    return *this;
}

char *sharpen::ByteVector::Data() noexcept {
    if (this->size_) {
        if (this->InlineBuffer()) {
            return this->rawVector_.inline_;
        }
        return this->rawVector_.external_.data_;
    }
    return nullptr;
}

const char *sharpen::ByteVector::Data() const noexcept {
    if (this->size_) {
        if (this->InlineBuffer()) {
            return this->rawVector_.inline_;
        }
        return this->rawVector_.external_.data_;
    }
    return nullptr;
}

char &sharpen::ByteVector::Get(std::size_t index) {
    if (index >= this->size_) {
        throw std::out_of_range("index out of range");
    }
    return this->Data()[index];
}

char sharpen::ByteVector::Get(std::size_t index) const {
    if (index >= this->size_) {
        throw std::out_of_range("index out of range");
    }
    return this->Data()[index];
}

void sharpen::ByteVector::Clear() noexcept {
    if (!this->InlineBuffer()) {
        char *p{nullptr};
        std::swap(p, this->rawVector_.external_.data_);
        if (p) {
            this->Free(p);
            this->rawVector_.external_.cap_ = 0;
        }
    }
    this->size_ = 0;
}

void sharpen::ByteVector::Resize(std::size_t newSize, char defalutVal) {
    if (!this->InlineBuffer(newSize)) {
        if (this->size_ < newSize) {
            if (this->InlineBuffer() || this->rawVector_.external_.cap_ < newSize) {
                std::size_t newCap{this->ComputeHeapSize(newSize)};
                if (newCap < sharpen::ByteVector::blobSize_) {
                    std::size_t sz{(std::max)(static_cast<std::size_t>(1), this->size_)};
                    newCap = (std::max)(sz * 2, newCap);
                }
                char *buf{reinterpret_cast<char *>(this->Alloc(newCap))};
                if (!buf) {
                    throw std::bad_alloc();
                }
                std::memcpy(buf, this->Data(), this->size_);
                if (!this->InlineBuffer()) {
                    this->Free(this->rawVector_.external_.data_);
                }
                for (std::size_t i = this->size_; i != newSize; ++i) {
                    buf[i] = defalutVal;
                }
                this->rawVector_.external_.data_ = buf;
                this->rawVector_.external_.cap_ = newCap;
            }
        }
    } else if (!this->InlineBuffer()) {
        char *p = this->rawVector_.external_.data_;
        std::memcpy(this->rawVector_.inline_, p, newSize);
        this->Free(p);
    } else if (this->size_ < newSize) {
        char *buf{this->rawVector_.inline_};
        for (std::size_t i = this->size_; i != newSize; ++i) {
            buf[i] = defalutVal;
        }
    }
    this->size_ = newSize;
}

void sharpen::ByteVector::Erase(std::size_t begin, std::size_t end) noexcept {
    assert(begin <= end);
    assert(end <= this->size_);
    if (begin != end) {
        std::size_t oldSize{this->size_};
        std::size_t size{end - begin};
        assert(oldSize >= size);
        std::size_t newSize{oldSize - size};
        if (!newSize) {
            return this->Clear();
        }
        if (!this->InlineBuffer() && this->InlineBuffer(newSize)) {
            char *p{this->rawVector_.external_.data_};
            if (begin) {
                std::memcpy(this->rawVector_.inline_, p, begin);
            }
            std::size_t moveSize{oldSize - end};
            if (moveSize) {
                std::memcpy(this->rawVector_.inline_ + begin, p + end, moveSize);
            }
            this->Free(p);
        } else {
            if (end == this->size_) {
                this->Resize(begin);
            } else {
                std::size_t moveSize{oldSize - end};
                if (moveSize) {
                    std::memcpy(this->Data() + begin, this->Data() + end, moveSize);
                }
            }
        }
        this->size_ = newSize;
    }
}

sharpen::ByteVector::Iterator sharpen::ByteVector::Begin() noexcept {
    return Iterator{this->Data()};
}

sharpen::ByteVector::Iterator sharpen::ByteVector::End() noexcept {
    char *p{this->Data()};
    if (p) {
        return Iterator{p + this->size_};
    }
    return Iterator{nullptr};
}

sharpen::ByteVector::ConstIterator sharpen::ByteVector::Begin() const noexcept {
    return ConstIterator{this->Data()};
}

sharpen::ByteVector::ConstIterator sharpen::ByteVector::End() const noexcept {
    const char *p{this->Data()};
    if (p) {
        return ConstIterator{p + this->size_};
    }
    return ConstIterator{nullptr};
}

sharpen::ByteVector::ReverseIterator sharpen::ByteVector::ReverseBegin() noexcept {
    char *p{this->Data()};
    if (p) {
        return ReverseIterator{p + this->size_ - 1};
    }
    return ReverseIterator{nullptr};
}

sharpen::ByteVector::ReverseIterator sharpen::ByteVector::ReverseEnd() noexcept {
    char *p{this->Data()};
    if (p) {
        return ReverseIterator{p - 1};
    }
    return ReverseIterator{nullptr};
}

sharpen::ByteVector::ConstReverseIterator sharpen::ByteVector::ReverseBegin() const noexcept {
    const char *p{this->Data()};
    if (p) {
        return ConstReverseIterator{p + this->size_ - 1};
    }
    return ConstReverseIterator{nullptr};
}

sharpen::ByteVector::ConstReverseIterator sharpen::ByteVector::ReverseEnd() const noexcept {
    const char *p{this->Data()};
    if (p) {
        return ConstReverseIterator{p - 1};
    }
    return ConstReverseIterator{nullptr};
}

bool sharpen::ByteVector::CheckPointer(const char *p) {
    return this->Data() && this->Data() <= p && this->Data() + this->GetSize() > p;
}

void sharpen::ByteVector::Erase(Iterator where) {
    assert(CheckPointer(where.GetPointer()));
    std::size_t whereInx{static_cast<std::size_t>(where.GetPointer() - this->Data())};
    this->Erase(whereInx);
}

void sharpen::ByteVector::Erase(Iterator begin, Iterator end) {
    assert(CheckPointer(begin.GetPointer()) && CheckPointer(end.GetPointer()));
    char *buf{this->Data()};
    std::size_t beginInx{static_cast<std::size_t>(begin.GetPointer() - buf)};
    std::size_t endInx{static_cast<std::size_t>(end.GetPointer() - buf)};
    this->Erase(beginInx, endInx);
}

void sharpen::ByteVector::Erase(ConstIterator where) {
    assert(CheckPointer(where.GetPointer()));
    std::size_t whereInx{static_cast<std::size_t>(where.GetPointer() - this->Data())};
    this->Erase(whereInx);
}

void sharpen::ByteVector::Erase(ConstIterator begin, ConstIterator end) {
    assert(CheckPointer(begin.GetPointer()) && CheckPointer(end.GetPointer()));
    char *buf{this->Data()};
    std::size_t beginInx{static_cast<std::size_t>(begin.GetPointer() - buf)};
    std::size_t endInx{static_cast<std::size_t>(end.GetPointer() - buf)};
    this->Erase(beginInx, endInx);
}