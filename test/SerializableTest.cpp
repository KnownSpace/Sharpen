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

    sharpen::Size ComputeSize() const noexcept
    {
        return Helper::ComputeSize(this->container_);
    }

    sharpen::Size LoadFrom(const char *data,sharpen::Size size)
    {
        return Helper::LoadFrom(this->container_,data,size);
    }

    sharpen::Size UnsafeStoreTo(char *data) const noexcept
    {
        Helper::ComputeSize(this->container_);
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
    return 0;
}