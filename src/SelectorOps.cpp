#include <stdexcept>
#include <memory>

#include <sharpen/SelectorOps.hpp>
#include <sharpen/IocpSelector.hpp>
#include <sharpen/EpollSelector.hpp>

sharpen::SelectorPtr sharpen::MakeDefaultSelector()
{
#ifdef SHARPEN_HAS_IOCP
    //use iocp
    return std::make_shared<sharpen::IocpSelector>();
#elif (defined (SHARPEN_HAS_EPOLL))
    //use epoll
    return std::make_shared<sharpen::EpollSelector>();
#else
    throw std::logic_error("no surported system");
#endif
}