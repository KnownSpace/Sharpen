#include <sharpen/HttpResponse.hpp>

#include <cstring>
#include <cassert>
#include <stdexcept>

#include <sharpen/ByteBuffer.hpp>
#include <sharpen/HttpParser.hpp>
#include <sharpen/HttpResponseDecoder.hpp>

sharpen::HttpResponse::HttpResponse()
    :version_(sharpen::HttpVersion::Unkown)
    ,status_(sharpen::HttpStatusCode::OK)
    ,header_()
    ,body_()
{}

sharpen::HttpResponse::HttpResponse(sharpen::HttpVersion version,sharpen::HttpStatusCode status)
    :version_(version)
    ,status_(status)
    ,header_()
    ,body_()
{}

sharpen::HttpResponse::HttpResponse(const Self &other)
    :version_(other.version_)
    ,status_(other.status_)
    ,header_(other.header_)
    ,body_(other.body_)
{}

sharpen::HttpResponse::HttpResponse(Self &&other) noexcept
    :version_(other.version_)
    ,status_(other.status_)
    ,header_(std::move(other.header_))
    ,body_(std::move(other.body_))
{}

sharpen::HttpResponse &sharpen::HttpResponse::operator=(const Self &other)
{
    sharpen::HttpResponse tmp(other);
    this->Swap(tmp);
    return *this;
}

sharpen::HttpResponse &sharpen::HttpResponse::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->version_ = other.version_;
        this->status_ = other.status_;
        this->header_ = std::move(other.header_);
        this->body_ = std::move(other.body_);
        other.version_ = sharpen::HttpVersion::Unkown;
        other.status_ = sharpen::HttpStatusCode::OK;
    }
    return *this;
}

void sharpen::HttpResponse::Swap(Self &other) noexcept
{
    if (&other != this)
    {
        std::swap(this->version_,other.version_);
        std::swap(this->status_,other.status_);
        this->header_.Swap(other.header_);
        this->body_.Swap(other.body_);
    }
}

void sharpen::HttpResponse::Clear()
{
    this->version_ = sharpen::HttpVersion::Unkown;
    this->status_ = sharpen::HttpStatusCode::OK;
    this->header_.Clear();
    this->body_.Clear();
}

std::size_t sharpen::HttpResponse::ComputeSize() const
{
    //version
    const char *name = sharpen::GetHttpVersionName(this->version_);
    std::size_t size = std::strlen(name);
    size += 1;
    //status code
    size += 3;
    size += 1;
    //status name
    name = sharpen::GetHttpStatusCodeName(this->status_);
    size += std::strlen(name);
    //CRLF
    size += 2;
    //header
    size += this->header_.ComputeSize();
    //body
    size += this->body_.GetSize();
    return size;
}

std::size_t sharpen::HttpResponse::CopyTo(char *buf,std::size_t size) const
{
    std::size_t needSize = this->ComputeSize();
    if (needSize > size)
    {
        throw std::length_error("buffer size less than need");
    }
    //version
    std::size_t offset = sharpen::CopyHttpVersionNameTo(this->version_,buf,size);
    buf[offset] = ' ';
    offset += 1;
    //status code
    int tmp = static_cast<int>(this->status_);
    buf[offset] = static_cast<char>(tmp/100 + 48);
    offset += 1;
    tmp %= 100;
    buf[offset] = static_cast<char>(tmp/10 +48);
    offset += 1;
    tmp %= 10;
    buf[offset] = static_cast<char>(tmp + 48);
    offset += 1;
    buf[offset] = ' ';
    offset += 1;
    //status name
    offset += sharpen::CopyHttpStatusCodeNameTo(this->status_,buf + offset,size - offset);
    //CRLF
    const char CRLF[] = "\r\n";
    std::memcpy(buf + offset,CRLF,sizeof(CRLF) - 1);
    offset += sizeof(CRLF) - 1;
    //header
    offset += this->header_.CopyTo(buf + offset,size - offset);
    //body
    this->body_.CopyTo(buf + offset,size - offset);
    return needSize;
}

std::size_t sharpen::HttpResponse::CopyTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    std::size_t size = this->ComputeSize();
    std::size_t left = buf.GetSize() - offset;
    if (size > left)
    {
        buf.Extend(size - left);
    }
    //version
    offset += sharpen::CopyHttpVersionNameTo(this->version_,buf,offset);
    buf[offset] = ' ';
    offset += 1;
    //status code
    int tmp = static_cast<int>(this->status_);
    buf[offset] = static_cast<char>(tmp/100 + 48);
    offset += 1;
    tmp %= 100;
    buf[offset] = static_cast<char>(tmp/10 +48);
    offset += 1;
    tmp %= 10;
    buf[offset] = static_cast<char>(tmp + 48);
    offset += 1;
    buf[offset] = ' ';
    offset += 1;
    //status name
    offset += sharpen::CopyHttpStatusCodeNameTo(this->status_,buf,offset);
    //CRLF
    const char CRLF[] = "\r\n";
    std::memcpy(buf.Data() + offset,CRLF,sizeof(CRLF) - 1);
    offset += sizeof(CRLF) - 1;
    //header
    offset += this->header_.CopyTo(buf,offset);
    //body
    this->body_.CopyTo(buf,offset);
    return size;
}

void sharpen::HttpResponse::ConfigParser(sharpen::HttpParser &parser)
{
    using Pair = std::pair<std::string,std::string>;
    using PairPtr = std::shared_ptr<Pair>;
    PairPtr headerBuffer = std::make_shared<Pair>(std::string(),std::string());
    parser.SetHeadersFieldCallback([headerBuffer](const char *data,std::size_t size) mutable
    {
        headerBuffer->first.assign(data,size);
        return 0;
    });
    parser.SetHeadersValueCallback([headerBuffer,this](const char *data,std::size_t size) mutable
    {
        headerBuffer->second.assign(data,size);
        this->Header().AddHeader(headerBuffer->first,headerBuffer->second);
        return 0;
    });
    parser.SetBodyCallback([this](const char *data,std::size_t size) mutable
    {
        this->Body().CopyFrom(data,size);
        return 0;
    });
    parser.SetHeadersCompleteCallback([this,&parser]() mutable
    {
        this->StatusCode() = parser.GetStatusCode();
        this->Version() = parser.GetVersion();
        return 0;
    });
    parser.SetMessageBeginCallback([&parser,this]() mutable
    {
        this->Clear();
        parser.SetCompleted(false);
        return 0;
    });
    parser.SetMessageEndCallback([&parser]() mutable
    {
        parser.SetCompleted(true);
        return 0;
    });
}