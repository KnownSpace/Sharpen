#include <sharpen/HttpServer.hpp>

#include <sharpen/HttpParser.hpp>

sharpen::HttpServer::HttpServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine,std::string name)
    :Mybase(af,endpoint,engine)
    ,name_(name)
{}

sharpen::HttpServer::HttpServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine)
    :HttpServer(af,endpoint,engine,"")
{}

void sharpen::HttpServer::OnNewChannel(sharpen::NetStreamChannelPtr channel)
{
    sharpen::ByteBuffer buf(4096);
    sharpen::HttpRequest req;
    sharpen::HttpResponse res;
    sharpen::HttpParser parser(sharpen::HttpParser::ParserModel::Request);
    req.ConfigParser(parser);
    bool keep = true;
    while (keep)
    {
        //parse request
        while (!parser.IsCompleted())
        {
            sharpen::Size n = channel->ReadAsync(buf);
            if (n == 0)
            {
                return;
            }
            parser.Parse(buf.Data(),n);
            if (parser.NeedUpgrade() || parser.IsError())
            {
                return;
            }
        }
        parser.SetCompleted(false);
        //set http version
        res.Version() = req.Version();
        //set server name
        if (!this->name_.empty())
        {
            res.Header()["Server"] = this->name_;
        }
        //set keep-alive
        if (parser.ShouldKeepalive())
        {
            res.Header()["Connection"] = "keep-alive";
        }
        else if(req.Version() != sharpen::HttpVersion::Http0_9)
        {
            res.Header()["Connection"] = "close";
        }
        keep = parser.ShouldKeepalive();
        //handle
        this->OnNewMessage(channel,req,res);
        //send res
        sharpen::Size n = res.CopyTo(buf);
        channel->WriteAsync(buf.Data(),n);
        //reset
        res.Clear();
        req.Clear();
    }
}