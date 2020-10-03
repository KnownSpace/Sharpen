#include <cstdio>

#include <sharpen/Future.hpp>

int main(int argc, char const *argv[])
{
    sharpen::Future<int> myfuture;
    myfuture.SetCallback([](sharpen::Future<int> &future)
    {
        int r = future.Get();
        std::printf("Hello World %d\n",r);
    });
    myfuture.Complete(0);
    return 0;
}
