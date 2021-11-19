#include <sharpen/MicroRpcField.hpp>
#include <sharpen/MicroRpcStack.hpp>

void VariableEncodeTest()
{
    std::printf("begin test\n");
    std::printf("single encode\n");
    {
        sharpen::MicroRpcVariable<sharpen::Int32> var(1);
        sharpen::MicroRpcField field{var};
        sharpen::MicroRpcFieldHeader &header = field.Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == 0);
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Int32));
        assert(*reinterpret_cast<sharpen::Int32*>(field.RawData().Data() + 1) == 1);
    }
    std::printf("pass\n");
    std::printf("collection encode\n");
    {
        const char arr[65535] = "this is code";
        sharpen::MicroRpcVariable<char> var{arr,arr + sizeof(arr)};
        sharpen::MicroRpcField field{var};
        sharpen::MicroRpcFieldHeader &header = field.Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == sizeof(sharpen::Uint16));
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Char));
        {
            sharpen::Size size = field.GetSize();
            assert(sizeof(arr) == size);
            std::printf("collection has %zu elements\n",size);
        }
        char *data = field.Data<char>();
        for (size_t i = 0; i < sizeof(arr); i++)
        {
            assert(arr[i] == data[i]);
        }
    }
    std::printf("pass\n");
}

void FieldEncodeTest()
{
    std::printf("begin test\n");
    std::printf("single encode\n");
    {
        sharpen::MicroRpcField field{1};
        sharpen::MicroRpcFieldHeader &header = field.Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == 0);
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Int32));
        assert(*reinterpret_cast<sharpen::Int32*>(field.RawData().Data() + 1) == 1);
    }
    std::printf("pass\n");
    std::printf("collection encode\n");
    {
        const char arr[65535] = "this is code";
        sharpen::MicroRpcField field{arr,arr + sizeof(arr)};
        sharpen::MicroRpcFieldHeader &header = field.Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == sizeof(sharpen::Uint16));
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Char));
        {
            sharpen::Size size = field.GetSize();
            assert(sizeof(arr) == size);
            std::printf("collection has %zu elements\n",size);
        }
        char *data = field.Data<char>();
        for (size_t i = 0; i < sizeof(arr); i++)
        {
            assert(arr[i] == data[i]);
        }
    }
    std::printf("pass\n");
}

void StackEncodeTest()
{
    std::printf("begin test\n");
    std::printf("single encode\n");
    {
        sharpen::MicroRpcStack stack;
        stack.Push(1);
        sharpen::MicroRpcFieldHeader &header = stack.Fields().front().Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == 0);
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Int32));
        assert(*reinterpret_cast<sharpen::Int32*>(stack.Fields().front().RawData().Data() + 1) == 1);
    }
    std::printf("pass\n");
    std::printf("collection encode\n");
    {
        const char arr[65535] = "this is code";
        sharpen::MicroRpcStack stack;
        stack.Push(arr,arr + sizeof(arr));
       sharpen::MicroRpcFieldHeader &header = stack.Fields().front().Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == sizeof(sharpen::Uint16));
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Char));
        {
            sharpen::Size size = stack.Fields().front().GetSize();
            assert(sizeof(arr) == size);
            std::printf("collection has %zu elements\n",size);
        }
        char *data = stack.Fields().front().Data<char>();
        for (size_t i = 0; i < sizeof(arr); i++)
        {
            assert(arr[i] == data[i]);
        }
    }
    std::printf("pass\n");
}

int main(int argc, char const *argv[])
{
    std::printf("encode variable test\n");
    VariableEncodeTest();
    std::printf("encode field test\n");
    FieldEncodeTest();
    std::printf("encode stack test\n");
    StackEncodeTest();
    return 0;
}