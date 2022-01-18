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
#include "HttpVersion.hpp"

struct llhttp__internal_s;

struct llhttp_settings_s;

namespace sharpen
{
    class ByteBuffer;

    class HttpParser:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::HttpParser;
        using EventCallback = std::function<int()>;
        using DataCallback = std::function<int(const char *,sharpen::Size)>;
        using Parser = llhttp__internal_s;
        using ParserSettings = llhttp_settings_s;

        Parser* parser_;
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
        bool completed_;

        static int OnMessageBegin(Parser *parser);

        static int OnUrl(Parser *parser,const char *str,sharpen::Size len);

        static int OnStatusCode(Parser *parser,const char *str,sharpen::Size len);

        static int OnHeadersField(Parser *parser,const char *str,sharpen::Size len);

        static int OnHeadersValue(Parser *parser,const char *str,sharpen::Size len);

        static int OnHeadersComplete(Parser *parser);

        static int OnBody(Parser *parser,const char *str,sharpen::Size len);
        
        static int OnMessageEnd(Parser *parser);

        static int OnChunkHeader(Parser *parser);

        static int OnChunkComplete(Parser *parser);

        static ParserSettings *GetSettings();
    public:
        enum class ParserModel
        {
            Both = 0,
            Request = 1,
            Response = 2
        };

        explicit HttpParser(ParserModel model);

        ~HttpParser() noexcept;

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

        void Parse(const char *data,sharpen::Size size);

        inline void Parse(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline void Parse(const sharpen::ByteBuffer &buf)
        {
            this->Parse(buf,0);
        }

        bool NeedUpgrade() const;

        bool ShouldKeepalive() const;

        sharpen::HttpMethod GetMethod() const;

        sharpen::HttpStatusCode GetStatusCode() const;

        sharpen::HttpVersion GetVersion() const;

        bool IsError() const;

        const char *GetErrorMessage() const;

        bool IsCompleted() const;

        void SetCompleted(bool completed);
    };
}

#endif