#include <cstdio>
#include <sharpen/IoUringQueue.hpp>

#ifdef SHARPEN_HAS_IOURING

#include <sys/uio.h>

#include <sharpen/EventEngine.hpp>
#include <sharpen/IFileChannel.hpp>

void Entry()
{
    if(!sharpen::TestIoUring())
    {
        std::puts("pass");
        return;
    }
    sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./iouring.test",sharpen::FileAccessMethod::All,sharpen::FileOpenMethod::CreateNew);
    sharpen::IoUringQueue queue{true};
    std::puts("write test");
    {
        io_uring_sqe sqe;
        sqe.opcode = IORING_OP_WRITEV;
        sqe.fd = channel->GetHandle();
        char buf[] = "Hello World";
        iovec vec;
        vec.iov_base = buf;
        vec.iov_len = sizeof(buf) - 1;
        sqe.addr = reinterpret_cast<std::uintptr_t>(&vec);
        sqe.len = 1;
        sqe.off = 0;
        queue.SubmitIoRequest(sqe);
        queue.EventFd().Read();
        io_uring_cqe cqe;
        queue.GetCompletionStatus(&cqe,1);
        std::printf("result is %d\n",cqe.res);
    }
    std::puts("read test");
    {
        io_uring_sqe sqe;
        sqe.opcode = IORING_OP_READV;
        sqe.fd = channel->GetHandle();
        char buf[16] = {0};
        iovec vec;
        vec.iov_base = buf;
        vec.iov_len = sizeof(buf);
        sqe.addr = reinterpret_cast<std::uintptr_t>(&vec);
        sqe.len = 1;
        sqe.off = 0;
        queue.SubmitIoRequest(sqe);
        queue.EventFd().Read();
        io_uring_cqe cqe;
        queue.GetCompletionStatus(&cqe,1);
        std::printf("result is %d\n",cqe.res);
    }
    std::puts("pass");
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}

#else
int main(int argc, char const *argv[])
{
    std::puts("pass");
    return 0;
}
#endif