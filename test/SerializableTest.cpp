#include <cstdio>
#include <map>

#include <sharpen/BinarySerializable.hpp>
#include <sharpen/CorruptedDataError.hpp>
#include <sharpen/Varint.hpp>

#include <simpletest/TestRunner.hpp>

class Message : public sharpen::BinarySerializable<Message> {
private:
    using Self = Message;
    using ContainerType = std::map<std::string, sharpen::Optional<std::string>>;

    ContainerType container_;

public:
    Message() = default;

    Message(const Self &other) = default;

    Message(Self &&other) noexcept = default;

    inline Self &operator=(const Self &other) {
        Self tmp{other};
        std::swap(tmp, *this);
        return *this;
    }

    inline Self &operator=(Self &&other) noexcept {
        if (this != std::addressof(other)) {
            this->container_ = std::move(other.container_);
        }
        return *this;
    }

    ~Message() noexcept = default;

    ContainerType &Container() noexcept {
        return this->container_;
    }

    const ContainerType &Container() const noexcept {
        return this->container_;
    }

    std::size_t ComputeSize() const noexcept {
        return Helper::ComputeSize(this->container_);
    }

    std::size_t LoadFrom(const char *data, std::size_t size) {
        return Helper::LoadFrom(this->container_, data, size);
    }

    std::size_t UnsafeStoreTo(char *data) const noexcept {
        return Helper::UnsafeStoreTo(this->container_, data);
    }
};

void Store(const sharpen::BinarySerializable<Message> &msg, sharpen::ByteBuffer &buf) {
    msg.StoreTo(buf);
}

void Load(sharpen::BinarySerializable<Message> &msg, const sharpen::ByteBuffer &buf) {
    msg.LoadFrom(buf);
}

class SimpleObjectTest : public simpletest::ITypenamedTest<SimpleObjectTest> {
private:
    using Self = SimpleObjectTest;

public:
    SimpleObjectTest() noexcept = default;

    ~SimpleObjectTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::ByteBuffer buf;
        {
            Message msg;
            msg.Container().emplace("hello", "world");
            msg.Container().emplace("1234", sharpen::EmptyOpt);
            Store(msg, buf);
        }
        {
            Message msg;
            Load(msg, buf);
            return this->Assert(
                msg.Container()["hello"].Get() == "world" && !msg.Container()["1234"].Exist(),
                "Serialize/Unserialize return wrong answer");
        }
    }
};

class StdContainerTest : public simpletest::ITypenamedTest<StdContainerTest> {
private:
    using Self = StdContainerTest;

public:
    StdContainerTest() noexcept = default;

    ~StdContainerTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::ByteBuffer buf;
        {
            std::vector<Message> msgs;
            for (std::size_t i = 0; i != 3; ++i) {
                Message msg;
                msg.Container().emplace("vector msg", "test");
                msgs.emplace_back(msg);
            }
            sharpen::BinarySerializator::StoreTo(msgs, buf);
        }
        {
            std::vector<Message> msgs;
            sharpen::BinarySerializator::LoadFrom(msgs, buf);
            bool status{true};
            for (std::size_t i = 0, count = msgs.size(); i != count; ++i) {
                auto &opt{msgs[i].Container()["vector msg"]};
                status = status && opt.Exist() && opt.Get() == "test";
            }
            return this->Assert(status, "Serialize/Unserialize return wrong answer");
        }
    }
};

int main(int argc, char const *argv[]) {
    simpletest::TestRunner runner;
    runner.Register<SimpleObjectTest>();
    runner.Register<StdContainerTest>();
    return runner.Run();
}