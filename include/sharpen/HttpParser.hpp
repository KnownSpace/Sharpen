#pragma once
#ifndef _SHARPEN_HTTPPARSER_HPP
#define _SHARPEN_HTTPPARSER_HPP

#include <memory>
#include <functional>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "TypeDef.hpp"
#include "HttpMethod.hpp"
#include "HttpStatusCode.hpp"

struct http_parser;

struct http_parser_settings;

namespace sharpen
{
    class HttpParser:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::HttpParser;
        using EventCallback = std::function<int()>;
        using DataCallback = std::function<int(const char *,sharpen::Size)>;

        std::unique_ptr<http_parser> parser_;
        EventCallback onMsgBegin_;
        DataCallback onUrl_;
        DataCallback onStatusCode_;
        DataCallback onHeadersField_;
        DataCallback onHeadersValue_;
        EventCallback onHeadersComplete_;
        DataCallback onBody_;
        EventCallback onMsgEnd_;
        EventCallback onChunkHeader_;
        EventCallback onChunkComplete_;

        static int OnMessageBegin(http_parser *parser);

        static int OnUrl(http_parser *parser,const char *str,sharpen::Size len);

        static int OnStatusCode(http_parser *parser,const char *str,sharpen::Size len);

        static int OnHeadersField(http_parser *parser,const char *str,sharpen::Size len);

        static int OnHeadersValue(http_parser *parser,const char *str,sharpen::Size len);

        static int OnHeadersComplete(http_parser *parser);

        static int OnBody(http_parser *parser,const char *str,sharpen::Size len);
        
        static int OnMessageEnd(http_parser *parser);

        static int OnChunkHeader(http_parser *parser);

        static int OnChunkComplete(http_parser *parser);

        static http_parser_settings *GetSettings();
    public:
        enum class ParserModel
        {
            Request,
            Response,
            Both
        };

        HttpParser(ParserModel model);

        ~HttpParser() noexcept = default;

        void SetMessageBeginCallback(EventCallback cb)
        {
            this->onMsgBegin_ = std::move(cb);
        }

        void SetUrlCallback(DataCallback cb)
        {
            this->onUrl_ = std::move(cb);
        }

        void SetStatusCodeCallback(DataCallback cb)
        {
            this->onStatusCode_ = std::move(cb);
        }

        void SetHeadersFieldCallback(DataCallback cb)
        {
            this->onHeadersField_ = std::move(cb);
        }

        void SetHeadersValueCallback(DataCallback cb)
        {
            this->onHeadersValue_ = std::move(cb);
        }

        void SetHeadersCompleteCallback(EventCallback cb)
        {
            this->onHeadersComplete_ = std::move(cb);
        }

        void SetBodyCallback(DataCallback cb)
        {
            this->onBody_ = std::move(cb);
        }

        void SetMessageEndCallback(EventCallback cb)
        {
            this->onMsgEnd_ = std::move(cb);
        }

        void SetChunkHeaderCallback(EventCallback cb)
        {
            this->onChunkHeader_ = std::move(cb);
        }

        void SetChunkCompleteCallback(EventCallback cb)
        {
            this->onChunkComplete_ = std::move(cb);
        }

        sharpen::Size Parse(const char *data,sharpen::Size size);

        bool NeedUpgrade() const;

        bool ShouldKeepalive() const;

        sharpen::HttpMethod GetMethod() const;

        sharpen::HttpStatusCode GetStatusCode() const;
    };
}

#endif