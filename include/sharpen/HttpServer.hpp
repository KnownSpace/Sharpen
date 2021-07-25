#pragma once
#ifndef _SHARPEN_HTTPSERVER_HPP
#define _SHARPEN_HTTPSERVER_HPP

#include "TcpServer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

namespace sharpen
{
    class HttpServer:public sharpen::TcpServer
    {
    private:
        using Mybase = sharpen::TcpServer;

        std::string name_;

    protected:
        virtual void DoHandleChannel(sharpen::NetStreamChannelPtr channel) override;

        virtual void DoHandleMessage(sharpen::NetStreamChannelPtr channel,const sharpen::HttpRequest &req,sharpen::HttpResponse &res) = 0;
    public:
        
        HttpServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine);

        HttpServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine,std::string name);

        virtual ~HttpServer() noexcept = default;
    };
}

#endif