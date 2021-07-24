#include <sharpen/PipeChannel.hpp>

#include <sharpen/PipeChannelImpl.hpp>

#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

void sharpen::MakePipeChannel(sharpen::InputPipeChannelPtr &in,sharpen::OutputPipeChannelPtr &out)
{
    sharpen::FileHandle inHandle;
    sharpen::FileHandle outHandle;
#ifdef SHARPEN_HAS_WININPUTPIPE
    BOOL r = ::CreatePipe(&inHandle,&outHandle,nullptr,0);
    if (r == FALSE)
    {
        sharpen::ThrowLastError();
    }
#else
    int fd[2];
    int r = ::pipe(fd);
    if(r == -1)
    {
        sharpen::ThrowLastError();
    }
    inHandle = fd[0];
    outHandle = fd[1];
#endif
    in = std::make_shared<sharpen::InputPipeChannelImpl>(inHandle);
    out = std::make_shared<sharpen::OutputPipeChannelImpl>(outHandle);
}