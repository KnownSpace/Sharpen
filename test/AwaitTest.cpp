#include <cstdio>
#include <cassert>

#include <sharpen/AsyncOps.hpp>
#include <sharpen/AwaitOps.hpp>

void AwaitTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([]()
    {
        std::printf("await test\n");
        auto f1 = sharpen::Async([]()
        {
            std::printf("hello ");
            return 1;
        });
        auto f2 = sharpen::Async([](){
            std::printf("world\n");
        });
        auto f3 = sharpen::Async([](){
            return "hello world";
        });
        int r;
        std::tie(r,std::ignore,std::ignore) = sharpen::AwaitAll(*f1,*f2,*f3);
        assert(r == 1);
        std::printf("ret %d\n",r);
        auto f4 = sharpen::Async([]()
        {
            sharpen::Delay(std::chrono::seconds(9));
            std::printf("ok1\n");
        });
        auto f5 = sharpen::Async([](){
            sharpen::Delay(std::chrono::seconds(1));
            std::printf("ok2\n");
        });
        sharpen::AwaitAny(*f5,*f4);
        std::printf("await test pass\n");
        std::printf("reset test begin\n");
        sharpen::AwaitableFuture<int> future;
        sharpen::Launch([&future](){
            future.Complete(2);
        });
        r = future.Await();
        assert(r == 2);
        future.Reset();
        sharpen::Launch([&future](){
            future.Complete(3);
        });
        r = future.Await();
        assert(r == 3);
        std::printf("reset test pass\n");
    });
}

int main()
{
    AwaitTest();
    return 0;
}
