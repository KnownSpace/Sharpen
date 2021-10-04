#include <cstdio>
#include <sharpen/Console.hpp>

void ConsoleTest()
{
    //print
    //output c style string
    sharpen::Print("hello world\n");
    //printf
    sharpen::Print("the number is ",1,"\n","float is ",1.00,"\n");
    //output ptr
    sharpen::Print("null pointer is ",nullptr,"\n");
    //output bool
    sharpen::Print("token is ",true,"\n");
    //%x & %X
    sharpen::Print("255 dec is ",sharpen::DecFormat<int>(255),"\n");
    sharpen::Print("255 hex is ",sharpen::HexFormat<int>(255),"\n");
    //0b
    sharpen::Print("255 bin is ",sharpen::BinFormat<int>(255),"\n");
    //output std::string
    std::string str{"std::string\n"};
    sharpen::Print(str);
}

int main()
{
    ConsoleTest();
    return 0;
}
