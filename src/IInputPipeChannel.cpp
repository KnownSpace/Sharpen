#include <sharpen/IInputPipeChannel.hpp>
#include <sharpen/PosixInputPipeChannel.hpp>
#include <sharpen/WinInputPipeChannel.hpp>

int sharpen::IInputPipeChannel::GetcharAsync()
{
    char buf;
    this->ReadAsync(&buf, 1);
    return buf;
}

std::size_t sharpen::IInputPipeChannel::GetsAsync(char *buf, std::size_t size)
{
    char *begin = buf;
    char *end = begin + size;
    while (begin != end)
    {
        int c = this->GetcharAsync();
#ifdef SHARPEN_IS_WIN
        if (c == '\r')
        {
            continue;
        }
#endif
        if (c == '\n')
        {
            *begin = '\0';
            return begin - buf;
        }
        ++begin;
    }
    return begin - buf;
}

std::string sharpen::IInputPipeChannel::GetsAsync()
{
    std::string str;
    int c = this->GetcharAsync();
    while (c != '\n')
    {
#ifdef SHARPEN_IS_WIN
        if (c == '\r')
        {
            c = this->GetcharAsync();
            continue;
        }
#endif
        str.push_back(static_cast<char>(c));
        c = this->GetcharAsync();
    }
    return str;
}

sharpen::InputPipeChannelPtr sharpen::OpenStdinPipe()
{
#ifdef SHARPEN_HAS_WININPUTPIPE
    sharpen::FileHandle handle = ::CreateFileA("CONIN$",
                                               FILE_GENERIC_READ,
                                               FILE_SHARE_READ,
                                               nullptr,
                                               OPEN_EXISTING,
                                               FILE_FLAG_OVERLAPPED,
                                               nullptr);
    if (handle == INVALID_HANDLE_VALUE)
    {
        sharpen::ThrowLastError();
    }
    return std::make_shared<sharpen::WinInputPipeChannel>(handle);
#else
    return std::make_shared<sharpen::PosixInputPipeChannel>(0);
#endif
}