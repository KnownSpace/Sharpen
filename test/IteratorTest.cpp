#include <vector>
#include <numeric>
#include <algorithm>
#include <sharpen/TwoWayIterator.hpp>
#include <sharpen/TypeTraits.hpp>
#include <sharpen/IteratorOps.hpp>

int main(int argc, char const *argv[])
{
    //[0,1,2] 3
    //[1,2,3] 6
    //[2,3,4] 9
    std::vector<std::vector<int>> vec;
    int checker[9] = {0,1,2,1,2,3,2,3,4};
    for (int i = 0; i != 3; ++i)
    {
        std::vector<int> tmp = {i,i+1,i+2};
        vec.push_back(tmp);
    }
    using SubIterator = std::vector<int>::iterator;
    using ParentIterator = std::vector<std::vector<int>>::iterator;
    using TwoWayIterator = sharpen::TwoWayIterator<ParentIterator,SubIterator>;
    {
        TwoWayIterator begin{vec.begin(),vec.end(),vec.begin()->begin()};
        TwoWayIterator end{vec.end(),vec.end(),SubIterator{}};
        sharpen::Size index{0};
        while (begin != end)
        {
            std::printf("inc %d\n",*begin);
            assert(*begin == checker[index++]);
            ++begin;
        }
    }
    {
        sharpen::Size index{8};
        TwoWayIterator begin{--vec.end(),vec.begin(),--(--vec.end())->end()};
        TwoWayIterator end{vec.begin(),vec.begin(),SubIterator{}};
        while (begin != end)
        {
            std::printf("dec %d\n",*begin);
            assert(*begin == checker[index--]);
            --begin;
        }
    }
    {
        TwoWayIterator begin{vec.begin(),vec.end(),vec.begin()->begin()};
        TwoWayIterator end{vec.end(),vec.end(),SubIterator{}};
        sharpen::Size size{sharpen::GetRangeSize(begin,end)};
        std::printf("size is %zu\n",size);
        assert(size == 9);
    }
    {
        std::vector<int> copy;
        TwoWayIterator begin{vec.begin(),vec.end(),vec.begin()->begin()};
        TwoWayIterator end{vec.end(),vec.end(),SubIterator{}};
        std::copy(begin,end,std::back_inserter(copy));
        for (size_t i = 0; i < 9; ++i)
        {
            assert(copy[i] == checker[i]);   
        }
    }
    {
        TwoWayIterator begin{vec.begin(),vec.end(),vec.begin()->begin()};
        TwoWayIterator end{vec.end(),vec.end(),SubIterator{}};
        std::printf("accumulate is %d\n",std::accumulate(begin,end,0));
    }
    return 0;
}