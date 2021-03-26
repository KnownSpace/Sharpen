#include <sharpen/IocpSelector.hpp>
#include <sharpen/EpollSelector.hpp>
#include <sharpen/KqueueSelector.hpp>

#include <stdexcept>
#include <memory>

sharpen::SelectorPtr sharpen::MakeDefaultSelector()
{
#ifdef SHARPEN_HAS_IOCP
    //use iocp
    return std::make_shared<sharpen::IocpSelector>();
#elif (defined (SHARPEN_HAS_EPOLL))
    //use epoll
    return std::make_shared<sharpen::EpollSelector>();
#elif (defined (SHARPEN_HAS_KQUEUE))
    //use kqueue
    return std::make_shared<sharpen::KqueueSelector>();
#else
    throw std::logic_error("no surported system");
#endif
}