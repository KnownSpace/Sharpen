#include <cstdio>
#include <map>
#include <sharpen/BinarySerializable.hpp>
#include <sharpen/Varint.hpp>
#include <sharpen/DataCorruptionException.hpp>

class Message:public sharpen::BinarySerializable<Message>
{
private:
    using Self = Message;
    using ContainerType = std::map<std::string,sharpen::Optional<std::string>>;

    ContainerType container_;
public:

    Message() = default;

    Message(const Self &other) = default;

    Message(Self &&other) noexcept = default;

    inline Self &operator=(const Self &other)
    {
        Self tmp{other};
        std::swap(tmp,*this);
        return *this;
    }

    inline Self &operator=(Self &&other) noexcept
    {
        if(this != std::addressof(other))
        {
            this->container_ = std::move(other.container_);
        }
        return *this;
    }

    ~Message() noexcept = default;

    ContainerType &Container() noexcept
    {
        return this->container_;
    }

    const ContainerType &Container() const noexcept
    {
        return this->container_;
    }

    std::size_t ComputeSize() const noexcept
    {
        return Helper::ComputeSize(this->container_);
    }

    std::size_t LoadFrom(const char *data,std::size_t size)
    {
        return Helper::LoadFrom(this->container_,data,size);
    }

    std::size_t UnsafeStoreTo(char *data) const noexcept
    {
        return Helper::UnsafeStoreTo(this->container_,data);
    }
};

void Store(const sharpen::BinarySerializable<Message> &msg,sharpen::ByteBuffer &buf)
{
    msg.StoreTo(buf);
}

void Load(sharpen::BinarySerializable<Message> &msg,const sharpen::ByteBuffer &buf)
{
    msg.LoadFrom(buf);
}

int main(int argc, char const *argv[])
{
    sharpen::ByteBuffer buf;
    {
        Message msg;
        msg.Container().emplace("hello","world");
        msg.Container().emplace("1234",sharpen::EmptyOpt);
        Store(msg,buf);
    }
    {
        Message msg;
        Load(msg,buf);
        for (auto begin = msg.Container().begin(),end = msg.Container().end(); begin != end; ++begin)
        {
            if(begin->second.Exist())
            {
                std::printf("%s:%s\n",begin->first.c_str(),begin->second.Get().c_str());
            }
        }
        assert(msg.Container()["hello"].Get() == "world");
        assert(!msg.Container()["1234"].Exist());
    }
    buf.Clear();
    {
        std::vector<Message> msgs;
        for (std::size_t i = 0; i != 3; ++i)
        {
            Message msg;
            msg.Container().emplace("vector msg","test");
            msgs.emplace_back(msg);   
        }
        sharpen::BinarySerializator::StoreTo(msgs,buf);
    }
    {
        std::vector<Message> msgs;
        sharpen::BinarySerializator::LoadFrom(msgs,buf);
        for (std::size_t i = 0,count = msgs.size(); i != count; ++i)
        {
            for (auto begin = msgs[i].Container().begin(),end = msgs[i].Container().end(); begin != end; ++begin)
            {
                if(begin->second.Exist())
                {
                    std::printf("%s:%s\n",begin->first.c_str(),begin->second.Get().c_str());
                }
            }
        }
    }
    buf.Clear();
    {
        sharpen::Optional<std::vector<Message>> msgs;
        msgs.Construct();
        for (std::size_t i = 0; i != 3; ++i)
        {
            Message msg;
            msg.Container().emplace("opt msgs","test");
            msgs.Get().emplace_back(msg);   
        }
        sharpen::BinarySerializator::StoreTo(msgs,buf);
    }
    {
        sharpen::Optional<std::vector<Message>> msgs;
        sharpen::BinarySerializator::LoadFrom(msgs,buf);
        for (std::size_t i = 0,count = msgs.Get().size(); i != count; ++i)
        {
            for (auto begin = msgs.Get()[i].Container().begin(),end = msgs.Get()[i].Container().end(); begin != end; ++begin)
            {
                if(begin->second.Exist())
                {
                    std::printf("%s:%s\n",begin->first.c_str(),begin->second.Get().c_str());
                }
            }
        }
    }
    return 0;
}