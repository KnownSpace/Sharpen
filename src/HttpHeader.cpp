#include <sharpen/HttpHeader.hpp>

#include <stdexcept>
#include <cstring>
#include <cassert>

#include <sharpen/ByteBuffer.hpp>

sharpen::HttpHeader::HttpHeader()
    :headers_()
{}

sharpen::HttpHeader::HttpHeader(const Self &other)
    :headers_(other.headers_)
{}

sharpen::HttpHeader::HttpHeader(Self &&other) noexcept
    :headers_(std::move(other.headers_))
{}

sharpen::HttpHeader &sharpen::HttpHeader::operator=(const Self &other)
{
    sharpen::HttpHeader tmp(other);
    this->Swap(tmp);
    return *this;
}

sharpen::HttpHeader &sharpen::HttpHeader::operator=(Self &&other) noexcept
{
    if(this == std::addressof(other))
    {
        return *this;
    }
    this->headers_ = std::move(other.headers_);
    return *this;
}

void sharpen::HttpHeader::AddHeader(std::string field,std::string value)
{
    this->headers_.emplace(std::move(field),std::move(value));
}

const std::string &sharpen::HttpHeader::GetHeader(const std::string &field) const
{
    return this->headers_.at(field);
}

std::string &sharpen::HttpHeader::GetHeader(const std::string &field)
{
    return this->headers_[field];
}

void sharpen::HttpHeader::RemoveHeader(const std::string &field)
{
    this->headers_.erase(field);   
}

bool sharpen::HttpHeader::ExistHeader(const std::string &field) const
{
    auto ite = this->headers_.find(field);
    return ite != this->headers_.cend();
}

std::size_t sharpen::HttpHeader::ComputeSize() const
{   
    std::size_t size{0};
    for (auto begin = this->headers_.begin();begin != this->headers_.end(); ++begin)
    {
        //field: value\r\n
        size += begin->first.size();
        size += begin->second.size();
        size += 4;
    }
    //CRLF in end
    size += 2;
    return size;
}

void sharpen::HttpHeader::CopyToMem(char *buf,std::size_t offset) const
{
    std::size_t i = offset;
    const char CRLF[] = "\r\n";
    const char div[] = ": ";
    for (auto begin = this->headers_.begin(); begin != this->headers_.end() ; begin++)
    {
        //field: value\r\n
        std::memcpy(buf + i,begin->first.data(),begin->first.size());
        i += begin->first.size();
        std::memcpy(buf + i,div,sizeof(div) - 1);
        i += sizeof(div) - 1;
        std::memcpy(buf + i,begin->second.data(),begin->second.size());
        i += begin->second.size();
        std::memcpy(buf + i,CRLF,sizeof(CRLF) - 1);
        i += sizeof(CRLF) - 1;
    }
    std::memcpy(buf + i,CRLF,sizeof(CRLF) - 1);
    //field: value\r\n
    //...
    //field: value\r\n\r\n
    assert((i + sizeof(CRLF) - offset - 1) == this->ComputeSize());
}

std::size_t sharpen::HttpHeader::CopyTo(char *buf,std::size_t size) const
{
    std::size_t needSize = this->ComputeSize();
    if (needSize > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    this->CopyToMem(buf,0);
    return needSize;
}

std::size_t sharpen::HttpHeader::CopyTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(offset <= buf.GetSize());
    std::size_t needSize = this->ComputeSize();
    std::size_t left = buf.GetSize() - offset;
    if (left < needSize)
    {
        buf.Extend(left - needSize);
    }
    this->CopyToMem(buf.Data(),offset);
    return needSize;
}

void sharpen::HttpHeader::Swap(sharpen::HttpHeader &other) noexcept
{
    if (&other != this)
    {
        std::swap(this->headers_,other.headers_);
    }
}

void sharpen::HttpHeader::Clear()
{
    this->headers_.clear();
}

sharpen::HttpHeader::ConstIterator sharpen::HttpHeader::Begin() const
{
    return this->headers_.cbegin();
}

sharpen::HttpHeader::Iterator sharpen::HttpHeader::Begin()
{
    return this->headers_.begin();
}

sharpen::HttpHeader::ConstIterator sharpen::HttpHeader::End() const
{
    return this->headers_.cend();
}

sharpen::HttpHeader::Iterator sharpen::HttpHeader::End()
{
    return this->headers_.end();
}