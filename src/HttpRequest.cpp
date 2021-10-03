#include <sharpen/HttpRequest.hpp>

#include <cstring>
#include <cassert>
#include <stdexcept>

#include <sharpen/ByteBuffer.hpp>
#include <sharpen/HttpParser.hpp>

sharpen::HttpRequest::HttpRequest()
    :HttpRequest(sharpen::HttpMethod::GET,"/",sharpen::HttpVersion::Unkown)
{}

sharpen::HttpRequest::HttpRequest(sharpen::HttpMethod method,std::string url,sharpen::HttpVersion version)
    :method_(method)
    ,url_(url)
    ,version_(version)
    ,header_()
    ,body_()
{}

sharpen::HttpRequest::HttpRequest(const Self &other)
    :method_(other.method_)
    ,url_(other.url_)
    ,version_(other.version_)
    ,header_(other.header_)
    ,body_(other.body_)
{}

sharpen::HttpRequest::HttpRequest(Self &&other) noexcept
    :method_(other.method_)
    ,url_(std::move(other.url_))
    ,version_(other.version_)
    ,header_(std::move(other.header_))
    ,body_(std::move(other.body_))
{}

sharpen::HttpRequest &sharpen::HttpRequest::operator=(const Self &other)
{
    sharpen::HttpRequest tmp(other);
    this->Swap(tmp);
    return *this;
}

sharpen::HttpRequest &sharpen::HttpRequest::operator=(Self &&other) noexcept
{
    if(this == std::addressof(other))
    {
        return *this;
    }
    this->method_ = other.method_;
    this->url_ = std::move(other.url_);
    this->version_ = other.version_;
    this->header_ = std::move(other.header_);
    this->body_ = std::move(other.body_);
    return *this;
}

void sharpen::HttpRequest::Swap(Self &other) noexcept
{
    if (&other != this)
    {
        std::swap(this->method_,other.method_);
        this->url_.swap(other.url_);
        std::swap(this->version_,other.version_);
        this->header_.swap(other.header_);
        this->body_.swap(other.body_);
    }
}

void sharpen::HttpRequest::Clear()
{
    this->method_ = sharpen::HttpMethod::GET;
    this->url_.clear();
    this->version_ = sharpen::HttpVersion::Unkown;
    this->header_.Clear();
    this->body_.Clear();
}

sharpen::Size sharpen::HttpRequest::ComputeSize() const
{
    sharpen::Size size{0};
    //method
    const char *name = sharpen::GetHttpMethodName(this->method_);
    size += std::strlen(name);
    size += 1;
    //url
    size += this->url_.size();
    size += 1;
    //version
    name = sharpen::GetHttpVersionName(this->version_);
    size += std::strlen(name);
    //CRLF
    size += 2;
    //header
    size += this->header_.ComputeSize();
    //body
    size += this->body_.GetSize();
    return size;
}

sharpen::Size sharpen::HttpRequest::CopyTo(char *buf,sharpen::Size size) const
{
    sharpen::Size needSize = this->ComputeSize();
    if (needSize > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    //copy method
    sharpen::Size offset{0};
    offset += sharpen::CopyHttpMethodNameTo(this->method_,buf,size);
    buf[offset] = ' ';
    offset += 1;
    //copy url
    std::memcpy(buf + offset,this->url_.c_str(),this->url_.size());
    offset += this->url_.size();
    buf[offset] = ' ';
    offset += 1;
    //copy version
    offset += sharpen::CopyHttpVersionNameTo(this->version_,buf,size - offset);
    //copy CRLF
    const char CRLF[] = "\r\n";
    std::memcpy(buf + offset,CRLF,sizeof(CRLF) -1);
    offset += sizeof(CRLF) - 1;
    //copy header
    offset += this->header_.CopyTo(buf + offset,size - offset);
    //copy body
    offset += this->body_.CopyTo(buf + offset,needSize - offset);
    assert(offset == needSize);
    return needSize;
}

sharpen::Size sharpen::HttpRequest::CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    sharpen::Size tmp = this->ComputeSize();
    if ((buf.GetSize() - offset) < tmp)
    {
        buf.Extend(tmp - buf.GetSize() - offset);
    }
    tmp = offset;
    //copy method
    offset += sharpen::CopyHttpMethodNameTo(this->method_,buf,offset);
    buf[offset] = ' ';
    offset += 1;
    //copy url
    std::memcpy(buf.Data() + offset,this->url_.c_str(),this->url_.size());
    offset += this->url_.size();
    buf[offset] = ' ';
    offset += 1;
    //copy version
    offset += sharpen::CopyHttpVersionNameTo(this->version_,buf,offset);
    //copy CRLF
    const char CRLF[] = "\r\n";
    std::memcpy(buf.Data() + offset,CRLF,sizeof(CRLF) - 1);
    offset += sizeof(CRLF) - 1;
    //copy header
    offset += this->header_.CopyTo(buf,offset);
    //copy body
    offset += this->body_.CopyTo(buf,offset);
    return offset - tmp;
}

void sharpen::HttpRequest::ConfigParser(sharpen::HttpParser &parser)
{
    using Pair = std::pair<std::string,std::string>;
    using PairPtr = std::shared_ptr<Pair>;
    PairPtr headerBuffer = std::make_shared<Pair>(std::string(),std::string());
    parser.SetHeadersFieldCallback([headerBuffer](const char *data,sharpen::Size size) mutable
    {
        headerBuffer->first.assign(data,size);
        return 0;
    });
    parser.SetHeadersValueCallback([headerBuffer,this](const char *data,sharpen::Size size) mutable
    {
        headerBuffer->second.assign(data,size);
        this->Header().AddHeader(headerBuffer->first,headerBuffer->second);
        return 0;
    });
    parser.SetBodyCallback([this](const char *data,sharpen::Size size) mutable
    {
        this->Body().CopyFrom(data,size);
        return 0;
    });
    parser.SetUrlCallback([this](const char *data,sharpen::Size size) mutable
    {
        this->Url().assign(data,size);
        return 0;
    });
    parser.SetHeadersCompleteCallback([this,&parser]() mutable
    {
        this->Method() = parser.GetMethod();
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