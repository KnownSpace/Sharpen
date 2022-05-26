#include <sharpen/AsyncOps.hpp>

//[begin,end)
void sharpen::ParallelFor(sharpen::Size begin,sharpen::Size end,sharpen::Size grainsSize,std::function<void(sharpen::Size)> fn)
{
    if (begin >= end)
    {
        return;
    }
    sharpen::Size parallelNumber{sharpen::EventEngine::GetEngine().LoopCount()};
    //compute max grainsSize
    sharpen::Size max {end - begin};
    max /= parallelNumber;
    if(max > grainsSize)
    {
        max = grainsSize;
    }
    //init iterator
    std::atomic_size_t ite{begin};
    //init future
    sharpen::AwaitableFuture<void> future;
    std::atomic_size_t comp{parallelNumber};
    //launch fiber
    for (sharpen::Size i = 0; i < parallelNumber; i++)
    {
        sharpen::Launch([&ite,max,end,parallelNumber,&fn,&comp,&future]()
        {
            while(true)
            {
                sharpen::Size iterator = ite.load();
                sharpen::Size size{0};
                //get size
                do
                {
                    assert(end >= iterator);
                    size = end - iterator;
                    if(size > max)
                    {
                        size = max;
                    }
                } while (!ite.compare_exchange_weak(iterator,iterator + size));
                if (!size)
                {
                    sharpen::Size copy = comp.fetch_sub(1);
                    copy -= 1;
                    if (copy == 0)
                    {
                        future.Complete();
                    }
                    return;
                }
                //execute function
                for (sharpen::Size i = 0; i < size; i++)
                {
                    assert(fn);
                    fn(i + iterator);
                }
            }
        });
    }
    future.Await();
}