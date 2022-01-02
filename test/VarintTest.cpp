#include <cstdio>
#include <sharpen/Varint.hpp>

int main(int argc, char const *argv[])
{
    std::puts("varint test begin");
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
        assert(i.Data() == comp);
    }
    std::puts("pass");
    return 0;
}
