#include <cstdio>
#include <sharpen/BinarySerializable.hpp>
#include <sharpen/Varint.hpp>
#include <sharpen/DataCorruptionException.hpp>

class Message:public sharpen::BinarySerializable<Message>
{
private:
    using Self = Message;

    std::vector<std::string> strs_;
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
            this->strs_ = std::move(other.strs_);
        }
        return *this;
    }

    ~Message() noexcept = default;

    std::vector<std::string> &Strings() noexcept
    {
        return this->strs_;
    }

    const std::vector<std::string> &Strings() const noexcept
    {
        return this->strs_;
    }

    sharpen::Size ComputeSize() const noexcept
    {
        return Helper::ComputeSize(this->strs_);
    }

    sharpen::Size LoadFrom(const char *data,sharpen::Size size)
    {
        return Helper::LoadFrom(this->strs_,data,size);
    }

    sharpen::Size UnsafeStoreTo(char *data) const noexcept
    {
        return Helper::UnsafeStoreTo(this->strs_,data);
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
        msg.Strings().emplace_back("hello");
        msg.Strings().emplace_back("world");
        Store(msg,buf);
    }
    {
        Message msg;
        Load(msg,buf);
        for (auto begin = msg.Strings().begin(),end = msg.Strings().end(); begin != end; ++begin)
        {
            std::puts(begin->c_str());
        }
        assert(msg.Strings()[0] == "hello");
        assert(msg.Strings()[1] == "world");
    }
    {
        sharpen::ByteBuffer tmp1{"123",3};
        sharpen::ByteBuffer tmpBuf;
        tmp1.StoreTo(tmpBuf);
        sharpen::ByteBuffer tmp2;
        tmp2.LoadFrom(tmpBuf);
        assert(tmp1 == tmp2);
    }
    return 0;
}