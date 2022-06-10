#include <cstdio>
#include <sharpen/AsyncOps.hpp>

void ParallelTest(size_t n)
{
    std::printf("parallel test begin\n");
    std::atomic_size_t ar{0};
    sharpen::ParallelFor(0, n, [&ar](size_t i)
    { 
        ar.fetch_add(i); 
    });
    size_t r{0};
    for (size_t i = 0; i < n; i++)
    {
        r += i;
    }
    std::printf("r is %zu ar is %zu\n", r, ar.load());
    assert(r == ar);
    std::printf("parallel test pass\n");
}

int main()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&ParallelTest,static_cast<std::size_t>(1e4));
    return 0;
}
