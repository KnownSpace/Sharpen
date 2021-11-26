#include <sharpen/Console.hpp>
#include <sharpen/IInputPipeChannel.hpp>
#include <sharpen/AsyncOps.hpp>

void Entry()
{
    sharpen::Print("welcome to command lines:\n");
    sharpen::InputPipeChannelPtr scaner = sharpen::MakeStdinPipe();
    scaner->Register(sharpen::EventEngine::GetEngine());
    sharpen::Launch([]()
    {
        sharpen::Print("waiting your input\n");
    });
    sharpen::Print("please input a line\n");
    std::string line = scaner->GetsAsync();
    sharpen::Print("your input is\n",line.c_str());
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}
