#include <memory>
#include <stdexcept>

#include <sharpen/EpollSelector.hpp>
#include <sharpen/IocpSelector.hpp>
#include <sharpen/SelectorOps.hpp>

sharpen::SelectorPtr sharpen::MakeDefaultSelector()
{
#ifdef SHARPEN_HAS_IOCP
    // use iocp
    return std::make_shared<sharpen::IocpSelector>();
#elif (defined(SHARPEN_HAS_EPOLL))
    // use epoll
    return std::make_shared<sharpen::EpollSelector>();
#else
    throw std::logic_error("no surported system");
#endif
}