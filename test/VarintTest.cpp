#include <cstdio>
#include <sharpen/Varint.hpp>

int main(int argc, char const *argv[])
{
    std::puts("varint test begin");
    {
        //-1
        sharpen::ByteBuffer comp{5};
        std::memset(comp.Data(),~0,comp.GetSize() - 1);
        comp.Back() = 120;
        {
            sharpen::Varint32 i{comp};
            std::printf("value is %d\n",i.Get());
            assert(i == -1);
        }
        {
            sharpen::Varint32 i{-1};
            assert(sharpen::BufferCompare(i.Data(),i.ComputeSize(),comp.Data(),comp.GetSize()) == 0);
        }
    }
    {
        //1
        sharpen::ByteBuffer comp{2};
        comp.Front() = -128;
        comp.Back() = 64;
        {
            sharpen::Varint32 i{comp};
            std::printf("value is %d\n",i.Get());
            assert(i == 1);
        }
        {
            sharpen::Varint32 i{1};
            assert(sharpen::BufferCompare(i.Data(),i.ComputeSize(),comp.Data(),comp.GetSize()) == 0);
        }
    }
    {
        //2
        sharpen::ByteBuffer comp{1};
        comp.Front() = 1;
        {
            sharpen::Varint32 i{comp};
            std::printf("value is %d\n",i.Get());
            assert(i == 2);
        }
        {
            sharpen::Varint32 i{2};
            assert(sharpen::BufferCompare(i.Data(),i.ComputeSize(),comp.Data(),comp.GetSize()) == 0);
        }
    }
    {
        //0
        sharpen::ByteBuffer comp{1};
        comp.Front() = 0;
        {
            sharpen::Varint32 i{comp};
            std::printf("value is %d\n",i.Get());
            assert(i == 0);
        }
        {
            sharpen::Varint32 i{0};
            assert(sharpen::BufferCompare(i.Data(),i.ComputeSize(),comp.Data(),comp.GetSize()) == 0);
        }
    }
    std::puts("pass");
    return 0;
}
