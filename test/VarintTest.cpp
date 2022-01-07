#include <cstdio>
#include <sharpen/Varint.hpp>

int main(int argc, char const *argv[])
{
    std::puts("varint test begin");
    {
        //-1
        sharpen::ByteBuffer comp{5};
        std::memset(comp.Data(),~0,comp.GetSize() - 1);
        comp.Back() = 15;
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
        sharpen::ByteBuffer comp{1};
        comp.Front() = 1;
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
        comp.Front() = 2;
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
    {
        //123456
        sharpen::ByteBuffer comp{3};
        comp[0] = -64;
        comp[1] = -60;
        comp[2] = 7;
        {
            sharpen::Varint32 i{comp};
            std::printf("value is %d\n",i.Get());
            assert(i == 123456);
        }
        {
            sharpen::Varint32 i{123456};
            assert(sharpen::BufferCompare(i.Data(),i.ComputeSize(),comp.Data(),comp.GetSize()) == 0);
        }
    }
    {
        //150
        sharpen::ByteBuffer comp{2};
        comp[0] = -106;
        comp[1] = 1;
        {
            sharpen::Varint32 i{comp};
            std::printf("value is %d\n",i.Get());
            assert(i == 150);
        }
        {
            sharpen::Varint32 i{150};
            assert(sharpen::BufferCompare(i.Data(),i.ComputeSize(),comp.Data(),comp.GetSize()) == 0);
        }
    }
    std::puts("pass");
    return 0;
}
