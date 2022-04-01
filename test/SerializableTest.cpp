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
        // sharpen::Varuint64 len{this->strs_.size()};
        // sharpen::Size size{0};
        // size += Helper::ComputeSize(len);
        // for (auto begin = this->strs_.begin(),end = this->strs_.end(); begin != end; ++begin)
        // {
        //     len.Set(begin->size());
        //     size += Helper::ComputeSize(len);
        //     size += begin->size();
        // }
        // return size;
        return Helper::ComputeSize(this->strs_);
    }

    sharpen::Size LoadFrom(const char *data,sharpen::Size size)
    {
        // if(!size)
        // {
        //     throw std::invalid_argument{"invalid buffer"};
        // }
        // sharpen::Varuint64 len{0};
        // sharpen::Size offset{0};
        // offset += Helper::LoadFrom(len,data,size);
        // for (sharpen::Size i = 0,count = len.Get(); i != count; ++i)
        // {
        //     if (size <= offset)
        //     {
        //         throw sharpen::DataCorruptionException("message corruption");
        //     }   
        //     offset += Helper::LoadFrom(len,data + offset,size - offset);
        //     std::string str{};
        //     str.resize(len.Get(),0);
        //     if (size < offset)
        //     {
        //         throw sharpen::DataCorruptionException("message corruption");
        //     }
        //     std::memcpy(const_cast<char*>(str.data()),data + offset,str.size());
        //     offset += str.size();
        //     this->strs_.emplace_back(std::move(str));   
        // }
        // return offset;
        return Helper::LoadFrom(this->strs_,data,size);
    }

    sharpen::Size UnsafeStoreTo(char *data) const noexcept
    {
        // sharpen::Size offset{0};
        // sharpen::Varuint64 len{this->strs_.size()};
        // offset += Helper::UnsafeStoreTo(len,data + offset);
        // for (auto begin = this->strs_.begin(),end = this->strs_.end(); begin != end; ++begin)
        // {
        //     len.Set(begin->size());
        
        //     offset += Helper::UnsafeStoreTo(len,data + offset);
        //     std::memcpy(data + offset,begin->data(),begin->size());
        //     offset += begin->size();
        // }
        // return offset;
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
    return 0;
}