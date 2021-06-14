#include <sharpen/PosixNetStreamChannel.hpp>

#ifdef SHARPEN_HAS_POSIXSOCKET

sharpen::PosixNetStreamChannel::PosixNetStreamChannel(sharpen::FileHandle handle)
    :Mybase()
{
    this->handle_ = handle;
}

void sharpen::PosixNetStreamChannel::WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{

}
        
void sharpen::PosixNetStreamChannel::WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future)
{

}

void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future)
{

}
        
void sharpen::PosixNetStreamChannel::ReadAsync(sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future)
{

}

void sharpen::PosixNetStreamChannel::OnEvent(sharpen::IoEvent *event)
{

}

void sharpen::PosixNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future)
{

}
        
void sharpen::PosixNetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future)
{

}

void sharpen::PosixNetStreamChannel::AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future)
{

}

void sharpen::PosixNetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint,sharpen::Future<void> &future)
{

}

#endif