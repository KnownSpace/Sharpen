#include <sharpen/HttpParser.hpp>

#include <stdexcept>
#include <mutex>
#include <cstdlib>
#include <llhttp.h>

sharpen::HttpParser::HttpParser(sharpen::HttpParser::ParserModel model)
    :parser_(nullptr)
    ,onMsgBegin_()
    ,onUrl_()
    ,onStatusCode_()
    ,onHeadersField_()
    ,onHeadersValue_()
    ,onHeadersComplete_()
    ,onBody_()
    ,onMsgEnd_()
    ,onChunkHeader_()
    ,onChunkComplete_()
    ,completed_(false)
{
    Parser *parser = reinterpret_cast<Parser*>(std::malloc(sizeof(Parser)));
    if (parser == nullptr)
    {
        throw std::bad_alloc();
    }
    this->parser_ = parser;
    ::llhttp_init(this->parser_,static_cast<llhttp_type>(model),sharpen::HttpParser::GetSettings());
    this->parser_->data = this;
}

sharpen::HttpParser::~HttpParser() noexcept
{
    std::free(this->parser_);
}

int sharpen::HttpParser::OnMessageBegin(Parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onMsgBegin_)
    {
        return thiz->onMsgBegin_();
    }
    return 0;
}

int sharpen::HttpParser::OnUrl(Parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onUrl_)
    {
        return thiz->onUrl_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnStatusCode(Parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onStatusCode_)
    {
        return thiz->onStatusCode_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnHeadersField(Parser *parser,const char *str,sharpen::Size len)
{
    if (len == 0)
    {
        return 0;
    }
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onHeadersField_)
    {
        return thiz->onHeadersField_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnHeadersValue(Parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onHeadersValue_)
    {
        return thiz->onHeadersValue_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnHeadersComplete(Parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onHeadersComplete_)
    {
        return thiz->onHeadersComplete_();
    }
    return 0;
}

int sharpen::HttpParser::OnBody(Parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onBody_)
    {
        return thiz->onBody_(str,len);
    }
    return 0;
}
        
int sharpen::HttpParser::OnMessageEnd(Parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onMsgEnd_)
    {
        return thiz->onMsgEnd_();
    }
    return 0;
}

int sharpen::HttpParser::OnChunkHeader(Parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onChunkHeader_)
    {
        return thiz->onChunkHeader_();
    }
    return 0;
}

int sharpen::HttpParser::OnChunkComplete(Parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onChunkComplete_)
    {
        return thiz->onChunkComplete_();
    }
    return 0;
}

sharpen::HttpParser::ParserSettings *sharpen::HttpParser::GetSettings()
{
    static ParserSettings settings;
    static std::once_flag flag;
    std::call_once(flag,[]() mutable
    {
        ::llhttp_settings_init(&settings);
        settings.on_message_begin = reinterpret_cast<llhttp_cb>(&sharpen::HttpParser::OnMessageBegin);
        settings.on_url = reinterpret_cast<llhttp_data_cb>(&sharpen::HttpParser::OnUrl);
        settings.on_status = reinterpret_cast<llhttp_data_cb>(&sharpen::HttpParser::OnStatusCode);
        settings.on_header_field = reinterpret_cast<llhttp_data_cb>(&sharpen::HttpParser::OnHeadersField);
        settings.on_header_value = reinterpret_cast<llhttp_data_cb>(&sharpen::HttpParser::OnHeadersValue);
        settings.on_headers_complete = reinterpret_cast<llhttp_cb>(&sharpen::HttpParser::OnHeadersComplete);
        settings.on_body = reinterpret_cast<llhttp_data_cb>(&sharpen::HttpParser::OnBody);
        settings.on_message_complete = reinterpret_cast<llhttp_cb>(&sharpen::HttpParser::OnMessageEnd);
        settings.on_chunk_header = reinterpret_cast<llhttp_cb>(&sharpen::HttpParser::OnChunkHeader);
        settings.on_chunk_complete = reinterpret_cast<llhttp_cb>(&sharpen::HttpParser::OnChunkComplete);
    });
    return &settings;
}

void sharpen::HttpParser::Parse(const char *data,sharpen::Size size)
{
    ::llhttp_execute(this->parser_,data,size);
}

bool sharpen::HttpParser::NeedUpgrade() const
{
    return this->parser_->upgrade;
}

bool sharpen::HttpParser::ShouldKeepalive() const
{
    return ::llhttp_should_keep_alive(this->parser_) != 0;
}

sharpen::HttpMethod sharpen::HttpParser::GetMethod() const
{
    int method = this->parser_->method;
    return static_cast<sharpen::HttpMethod>(method);
}

sharpen::HttpStatusCode sharpen::HttpParser::GetStatusCode() const
{
    int code = this->parser_->status_code;
    return static_cast<sharpen::HttpStatusCode>(code);
}

sharpen::HttpVersion sharpen::HttpParser::GetVersion() const
{
    return sharpen::GetHttpVersion(this->parser_->http_major,this->parser_->http_minor);
}

bool sharpen::HttpParser::IsError() const
{
    return this->parser_->error != llhttp_errno::HPE_OK;
}

const char *sharpen::HttpParser::GetErrorMessage() const
{
    return llhttp_errno_name(static_cast<llhttp_errno>(this->parser_->error));
}

bool sharpen::HttpParser::IsCompleted() const
{
    return this->completed_;
}

void sharpen::HttpParser::SetCompleted(bool completed)
{
    this->completed_ = completed;
}