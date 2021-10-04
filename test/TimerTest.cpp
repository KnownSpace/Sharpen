#include <cstdio>
#include <sharpen/TimeWheel.hpp>

void TimeWheelTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([]()
    {
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        sharpen::TimeWheel wheel(std::chrono::seconds(1),10,timer);
        sharpen::TimeWheelPtr upstream = std::make_shared<sharpen::TimeWheel>(std::chrono::seconds(10),6);
        bool token = false;
        upstream->Put(std::chrono::seconds(10),[&wheel,&token]() mutable
        {
            assert(token);
            wheel.Stop();
        });
        wheel.SetUpstream(upstream);
        wheel.Put(std::chrono::seconds(9),[&wheel,&token]() mutable
        {
            std::printf("set token true\n");
            token = true;
        });
        wheel.RunAsync();
    });
}

int main(int argc, char const *argv[])
{
    TimeWheelTest();
    return 0;
}
