#include <sharpen/MicroRpcField.hpp>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/MicroRpcDecoder.hpp>

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
        sharpen::MicroRpcFieldHeader &header = stack.Top().Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == 0);
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Int32));
        assert(*reinterpret_cast<sharpen::Int32*>(stack.Top().RawData().Data() + 1) == 1);
    }
    std::printf("pass\n");
    std::printf("collection encode\n");
    {
        const char arr[65535] = "this is code";
        sharpen::MicroRpcStack stack;
        stack.Push(arr,arr + sizeof(arr));
       sharpen::MicroRpcFieldHeader &header = stack.Top().Header();
        assert(header.end_ == 0);
        assert(header.sizeSpace_ == sizeof(sharpen::Uint16));
        assert(header.type_ == static_cast<unsigned char>(sharpen::MicroRpcVariableType::Char));
        {
            sharpen::Size size = stack.Top().GetSize();
            assert(sizeof(arr) == size);
            std::printf("collection has %zu elements\n",size);
        }
        char *data = stack.Top().Data<char>();
        for (size_t i = 0; i < sizeof(arr); i++)
        {
            assert(arr[i] == data[i]);
        }
    }
    std::printf("pass\n");
}

void DecodeVariableTest()
{
    sharpen::ByteBuffer buf;
    {
        sharpen::MicroRpcStack stack;
        stack.Push(1);
        stack.CopyTo(buf);
    }
    sharpen::MicroRpcStack stack;
    sharpen::MicroRpcDecoder decoder;
    decoder.Bind(stack);
    decoder.Decode(buf.Data(),buf.GetSize());
    for (size_t i = 0; i < buf.GetSize(); i++)
    {
        assert(stack.Top().RawData()[i] == buf[i]);
    }
    std::printf("pass\n");
}

void DecodeCollectionTest()
{
    sharpen::ByteBuffer buf;
    {
        sharpen::MicroRpcStack stack;
        const char str[] = "Hello";
        stack.Push(str,str + sizeof(str));
        stack.CopyTo(buf);
    }
    sharpen::MicroRpcStack stack;
    sharpen::MicroRpcDecoder decoder;
    decoder.Bind(stack);
    decoder.Decode(buf.Data(),buf.GetSize());
    for (size_t i = 0; i < buf.GetSize(); i++)
    {
        assert(stack.Top().RawData()[i] == buf[i]);
    }
    std::printf("pass\n");
}

void MultiDecodeTest()
{
    sharpen::ByteBuffer buf;
    {
        sharpen::MicroRpcStack stack;
        const char str[] = "Hello";
        stack.Push(str,str + sizeof(str));
        stack.Push(1);
        stack.CopyTo(buf);
    }
    sharpen::MicroRpcStack stack;
    sharpen::MicroRpcDecoder decoder;
    decoder.Bind(stack);
    decoder.Decode(buf.Data(),buf.GetSize());
    const char *data = buf.Data();
    for (auto begin = stack.Begin(),end = stack.End();begin != end;++begin)
    {
        for (size_t i = 0; i < begin->GetRawSize(); i++)
        {
            assert(*data == begin->RawData()[i]);
            ++data;   
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
    std::printf("decode variable test\n");
    DecodeVariableTest();
    std::printf("decode collection test\n");
    DecodeCollectionTest();
    std::printf("multi decode test\n");
    MultiDecodeTest();
    return 0;
}