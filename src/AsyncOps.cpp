#include <sharpen/AsyncOps.hpp>

//[begin,end)
void sharpen::ParallelFor(sharpen::Size begin,sharpen::Size end,sharpen::Size grainsSize,std::function<void(sharpen::Size)> fn)
{
    if (begin >= end)
    {
        return;
    }
    sharpen::Size pNum = sharpen::EventEngine::GetEngine().LoopNumber();
    //compute max grainsSize
    sharpen::Size max = end - begin;
    max /= pNum;
    if(max > grainsSize)
    {
        max = grainsSize;
    }
    //init iterator
    std::atomic_size_t ite{begin};
    std::atomic_size_t comp{begin};
    //init future
    sharpen::AwaitableFuture<void> future;
    //launch fiber
    for (sharpen::Size i = 0; i < pNum; i++)
    {
        sharpen::Launch([&ite,&comp,max,end,pNum,fn,&future]()
        {
            while(true)
            {
                sharpen::Size iterator = ite.load();
                sharpen::Size size{0};
                //interlocked
                do
                {
                    if (iterator >= end)
                    {
                        return;
                    }
                    size = end - iterator;
                    if(size > max)
                    {
                        size = max;
                    }
                } while (!ite.compare_exchange_strong(iterator,iterator + size));
                //execute function
                for (sharpen::Size i = 0; i < size; i++)
                {
                    fn(i + iterator);
                }
                //complete
                sharpen::Size ccomp = comp.fetch_add(size);
                ccomp += size;
                if (ccomp >= end)
                {
                    future.Complete();
                    return;
                }
            }
        });
    }
    future.Await();
}