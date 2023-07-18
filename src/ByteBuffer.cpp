#include <sharpen/ByteBuffer.hpp>

#include <sharpen/CorruptedDataError.hpp>
#include <sharpen/IntOps.hpp>
#include <sharpen/Varint.hpp>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>

sharpen::ByteBuffer::ByteBuffer(std::size_t size)
    : vector_(size) {
}

sharpen::ByteBuffer::ByteBuffer(sharpen::ByteSlice slice)
    : Self{slice.Data(), slice.GetSize()} {
}

sharpen::ByteBuffer::ByteBuffer(const char *p, std::size_t size)
    : vector_(size) {
    if (size) {
        std::memcpy(this->vector_.Data(), p, size);
    }
}

void sharpen::ByteBuffer::PushBack(char val) {
    this->vector_.PushBack(val);
}

std::size_t sharpen::ByteBuffer::GetSize() const noexcept {
    return this->vector_.GetSize();
}

void sharpen::ByteBuffer::PopBack() {
    this->vector_.PopBack();
}

char sharpen::ByteBuffer::Back() const {
    return this->vector_.Back();
}

char &sharpen::ByteBuffer::Back() {
    return this->vector_.Back();
}

char sharpen::ByteBuffer::Front() const {
    return this->vector_.Front();
}

char &sharpen::ByteBuffer::Front() {
    return this->vector_.Front();
}

char sharpen::ByteBuffer::Get(std::size_t index) const {
    return this->vector_.Get(index);
}

char &sharpen::ByteBuffer::Get(std::size_t index) {
    return this->vector_.Get(index);
}

const char *sharpen::ByteBuffer::Data() const noexcept {
    return this->vector_.Data();
}

char *sharpen::ByteBuffer::Data() noexcept {
    return reinterpret_cast<char *>(this->vector_.Data());
}

void sharpen::ByteBuffer::Extend(std::size_t size, char defaultValue) {
    this->vector_.Resize(this->GetSize() + size, defaultValue);
}

void sharpen::ByteBuffer::Extend(std::size_t size) {
    this->Extend(size, 0);
}

void sharpen::ByteBuffer::ExtendTo(std::size_t size, char defaultValue) {
    this->vector_.Resize(size, defaultValue);
}

void sharpen::ByteBuffer::ExtendTo(std::size_t size) {
    this->ExtendTo(size, 0);
}

void sharpen::ByteBuffer::Reset() noexcept {
    if (this->Empty()) {
        return;
    }
    std::memset(this->Data(), 0, this->GetSize());
}

void sharpen::ByteBuffer::Append(const char *p, std::size_t size) {
    if (!size) {
        return;
    }
    std::size_t oldSize{this->GetSize()};
    this->Extend(size);
    for (std::size_t i = oldSize, newSize = this->GetSize(); i != newSize; ++i) {
        this->Get(i) = *p++;
    }
}

void sharpen::ByteBuffer::Append(const sharpen::ByteBuffer &other) {
    this->Append(other.Data(), other.GetSize());
}

void sharpen::ByteBuffer::Erase(std::size_t pos) {
    this->vector_.Erase(pos);
}

void sharpen::ByteBuffer::Erase(std::size_t begin, std::size_t end) {
    this->vector_.Erase(begin, end);
}

sharpen::ByteBuffer::Iterator sharpen::ByteBuffer::Find(char e) noexcept {
    auto begin = this->Begin();
    auto end = this->End();
    while (begin != end) {
        if (*begin == e) {
            return begin;
        }
        ++begin;
    }
    return begin;
}

sharpen::ByteBuffer::ConstIterator sharpen::ByteBuffer::Find(char e) const noexcept {
    auto begin = this->Begin();
    auto end = this->End();
    while (begin != end) {
        if (*begin == e) {
            return begin;
        }
        ++begin;
    }
    return begin;
}

sharpen::ByteBuffer::ReverseIterator sharpen::ByteBuffer::ReverseFind(char e) noexcept {
    auto begin = this->ReverseBegin();
    auto end = this->ReverseEnd();
    while (begin != end) {
        if (*begin == e) {
            return begin;
        }
        ++begin;
    }
    return begin;
}

sharpen::ByteBuffer::ConstReverseIterator sharpen::ByteBuffer::ReverseFind(char e) const noexcept {
    auto begin = this->ReverseBegin();
    auto end = this->ReverseEnd();
    while (begin != end) {
        if (*begin == e) {
            return begin;
        }
        ++begin;
    }
    return begin;
}

void sharpen::ByteBuffer::Erase(ConstIterator where) {
    this->vector_.Erase(where);
}

void sharpen::ByteBuffer::Erase(ConstIterator begin, ConstIterator end) {
    this->vector_.Erase(begin, end);
}

char sharpen::ByteBuffer::GetOrDefault(std::size_t index, char defaultVal) const noexcept {
    if (index >= this->GetSize()) {
        return defaultVal;
    }
    return this->Get(index);
}

std::size_t sharpen::ByteBuffer::ComputeSize() const noexcept {
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->GetSize()};
    offset += builder.ComputeSize();
    offset += this->GetSize();
    return offset;
}

std::size_t sharpen::ByteBuffer::LoadFrom(const char *data, std::size_t size) {
    std::size_t offset{0};
    sharpen::Varuint64 builder{data, size};
    std::size_t sz{builder.ComputeSize()};
    offset += sz;
    sz = sharpen::IntCast<std::size_t>(builder.Get());
    if (sz) {
        if (size < offset + sz) {
            this->Clear();
            throw sharpen::CorruptedDataError("byte buffer corruption");
        }
        this->ExtendTo(sz);
        std::memcpy(this->Data(), data + offset, sz);
        offset += sz;
    }
    return offset;
}

std::size_t sharpen::ByteBuffer::LoadFrom(const sharpen::ByteBuffer &buf, std::size_t offset) {
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset, buf.GetSize() - offset);
}

std::size_t sharpen::ByteBuffer::UnsafeStoreTo(char *data) const noexcept {
    std::size_t offset{0};
    sharpen::Varuint64 builder{this->GetSize()};
    std::size_t size{builder.ComputeSize()};
    std::memcpy(data, builder.Data(), size);
    offset += size;
    std::memcpy(data + offset, this->Data(), this->GetSize());
    offset += this->GetSize();
    return offset;
}

std::size_t sharpen::ByteBuffer::StoreTo(char *data, std::size_t size) const {
    std::size_t needSize{this->ComputeSize()};
    if (needSize > size) {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::ByteBuffer::StoreTo(sharpen::ByteBuffer &buf, std::size_t offset) const {
    assert(buf.GetSize() >= offset);
    std::size_t size{buf.GetSize() - offset};
    std::size_t needSize{this->ComputeSize()};
    if (needSize > size) {
        buf.ExtendTo(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

sharpen::ByteSlice sharpen::ByteBuffer::GetSlice(std::size_t index, std::size_t size) const {
    if (index + size > this->GetSize()) {
        throw std::out_of_range{"index out of range"};
    }
    return {this->Data() + index, size};
}

sharpen::ByteSlice sharpen::ByteBuffer::GetSlice(ConstIterator begin, ConstIterator end) const {
    std::size_t index{sharpen::GetRangeSize(this->Begin(), begin)};
    std::size_t size{sharpen::GetRangeSize(begin, end)};
    return this->GetSlice(index, size);
}

int sharpen::ByteBuffer::PrintfNoexcept(const char *format, ...) noexcept {
    std::va_list args;
    std::va_list copyArgs;
    va_start(args, format);
    va_start(copyArgs,format);
    std::size_t sz{sharpen::IntCast<std::size_t>(std::vsnprintf(nullptr,0,format,args))};
    int result{0};
    if (this->GetSize() > sz) {
        result = std::vsnprintf(this->Data(), this->GetSize(), format, copyArgs);
    }
    va_end(args);
    va_end(copyArgs);
    return result;
}

void sharpen::ByteBuffer::Printf(const char *format, ...) {
    std::va_list args;
    std::va_list copyArgs;
    va_start(args, format);
    va_start(copyArgs,format);
    std::size_t sz{sharpen::IntCast<std::size_t>(std::vsnprintf(nullptr,0,format,args))};
    try {
        this->ExtendTo(sz + 1);
    } catch (const std::bad_alloc &rethrow) {
        (void)rethrow;
        va_end(args);
        va_end(copyArgs);
        throw;
    }
    int result{std::vsnprintf(this->Data(), this->GetSize(), format, copyArgs)};
    assert(sharpen::IntCast<std::size_t>(result) == sz);
    (void)result;
    va_end(args);
    va_end(copyArgs);
}

int sharpen::ByteBuffer::Scanf(const char *format, ...) noexcept {
    if (this->Empty()) {
        return EOF;
    }
    if (this->Back() != '\0') {
        return EOF;
    }
    std::va_list args;
    va_start(args, format);
    int result{std::vsscanf(this->Data(), format, args)};
    va_end(args);
    return result;
}