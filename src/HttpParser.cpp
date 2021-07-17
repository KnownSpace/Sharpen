#include <sharpen/HttpParser.hpp>

#include <stdexcept>
#include <mutex>
#include <cstdlib>
#include <http_parser.h>

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
{
    http_parser *parser = reinterpret_cast<http_parser*>(std::malloc(sizeof(http_parser)));
    if (parser == nullptr)
    {
        throw std::bad_alloc();
    }
    this->parser_.reset(parser);
    ::http_parser_init(this->parser_.get(),static_cast<http_parser_type>(model));
    this->parser_->data = this;
}

int sharpen::HttpParser::OnMessageBegin(http_parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onMsgBegin_)
    {
        return thiz->onMsgBegin_();
    }
    return 0;
}

int sharpen::HttpParser::OnUrl(http_parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onUrl_)
    {
        return thiz->onUrl_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnStatusCode(http_parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onStatusCode_)
    {
        return thiz->onStatusCode_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnHeadersField(http_parser *parser,const char *str,sharpen::Size len)
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

int sharpen::HttpParser::OnHeadersValue(http_parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onHeadersValue_)
    {
        return thiz->onHeadersValue_(str,len);
    }
    return 0;
}

int sharpen::HttpParser::OnHeadersComplete(http_parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onHeadersComplete_)
    {
        return thiz->onHeadersComplete_();
    }
    return 0;
}

int sharpen::HttpParser::OnBody(http_parser *parser,const char *str,sharpen::Size len)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onBody_)
    {
        return thiz->onBody_(str,len);
    }
    return 0;
}
        
int sharpen::HttpParser::OnMessageEnd(http_parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onMsgEnd_)
    {
        return thiz->onMsgEnd_();
    }
    return 0;
}

int sharpen::HttpParser::OnChunkHeader(http_parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onChunkHeader_)
    {
        return thiz->onChunkHeader_();
    }
    return 0;
}

int sharpen::HttpParser::OnChunkComplete(http_parser *parser)
{
    sharpen::HttpParser *thiz = reinterpret_cast<sharpen::HttpParser*>(parser->data);
    if (thiz->onChunkComplete_)
    {
        return thiz->onChunkComplete_();
    }
    return 0;
}

http_parser_settings *sharpen::HttpParser::GetSettings()
{
    static http_parser_settings settings;
    static std::once_flag flag;
    std::call_once(flag,[]() mutable
    {
        ::http_parser_settings_init(&settings);
        settings.on_message_begin = reinterpret_cast<http_cb>(&sharpen::HttpParser::OnMessageBegin);
        settings.on_url = reinterpret_cast<http_data_cb>(&sharpen::HttpParser::OnUrl);
        settings.on_status = reinterpret_cast<http_data_cb>(&sharpen::HttpParser::OnStatusCode);
        settings.on_header_field = reinterpret_cast<http_data_cb>(&sharpen::HttpParser::OnHeadersField);
        settings.on_header_value = reinterpret_cast<http_data_cb>(&sharpen::HttpParser::OnHeadersValue);
        settings.on_headers_complete = reinterpret_cast<http_cb>(&sharpen::HttpParser::OnHeadersComplete);
        settings.on_body = reinterpret_cast<http_data_cb>(&sharpen::HttpParser::OnBody);
        settings.on_message_complete = reinterpret_cast<http_cb>(&sharpen::HttpParser::OnMessageEnd);
        settings.on_chunk_header = reinterpret_cast<http_cb>(&sharpen::HttpParser::OnChunkHeader);
        settings.on_chunk_complete = reinterpret_cast<http_cb>(&sharpen::HttpParser::OnChunkComplete);
    });
}

sharpen::Size sharpen::HttpParser::Parse(const char *data,sharpen::Size size)
{
    return ::http_parser_execute(this->parser_.get(),sharpen::HttpParser::GetSettings(),data,size);
}

bool sharpen::HttpParser::NeedUpgrade() const
{
    return this->parser_->upgrade;
}

bool sharpen::HttpParser::ShouldKeepalive() const
{
    return ::http_should_keep_alive(this->parser_.get());
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