#include <sharpen/Process.hpp>


#include <sharpen/PosixInputPipeChannel.hpp>
#include <sharpen/PosixOutputPipeChannel.hpp>
#include <sharpen/ProcessOps.hpp>
#include <sharpen/WinInputPipeChannel.hpp>
#include <sharpen/WinOutputPipeChannel.hpp>
#include <sharpen/YieldOps.hpp>

#ifdef SHARPEN_IS_WIN
#include <sharpen/WinEx.h>
#endif

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <cassert>
#include <cstring>

#ifdef SHARPEN_IS_WIN
static HANDLE invalidHandle{INVALID_HANDLE_VALUE};
#else
static constexpr int invalidHandle{-1};
#endif

sharpen::Process::Process(std::string name,
                          std::string workDirectory,
                          std::vector<std::string> args)
    : name_(std::move(name))
    , workDirectory_(std::move(workDirectory))
    , args_(std::move(args))
    , handle_(invalidHandle)
    , stdin_(invalidHandle)
    , stdout_(invalidHandle)
    , stderr_(invalidHandle)
{
    assert(!this->name_.empty());
}

sharpen::Process::Process(std::string name, std::string workDirectory)
    : name_(std::move(name))
    , workDirectory_(std::move(workDirectory))
    , args_()
    , handle_(invalidHandle)
    , stdin_(invalidHandle)
    , stdout_(invalidHandle)
    , stderr_(invalidHandle)
{
    assert(!this->name_.empty());
}

sharpen::Process::Process(Self &&other) noexcept
    : name_(std::move(other.name_))
    , workDirectory_(std::move(other.workDirectory_))
    , args_(std::move(other.args_))
    , handle_(other.handle_)
    , stdin_(other.stdin_)
    , stdout_(other.stdout_)
    , stderr_(other.stderr_)
{
    other.handle_ = invalidHandle;
    other.stdin_ = invalidHandle;
    other.stdout_ = invalidHandle;
    other.stderr_ = invalidHandle;
}

void sharpen::Process::ReleaseHandles() noexcept
{
    if (this->stdin_ != invalidHandle)
    {
        sharpen::CloseFileHandle(this->stdin_);
        this->stdin_ = invalidHandle;
    }
    if (this->stdout_ != invalidHandle)
    {
        sharpen::CloseFileHandle(this->stdout_);
        this->stdout_ = invalidHandle;
    }
    if (this->stderr_ != invalidHandle)
    {
        sharpen::CloseFileHandle(this->stderr_);
        this->stderr_ = invalidHandle;
    }
    if (this->handle_ != invalidHandle)
    {
#ifdef SHARPEN_IS_WIN
        sharpen::CloseFileHandle(this->handle_);
#endif
        this->handle_ = invalidHandle;
    }
}

sharpen::Process &sharpen::Process::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->name_ = std::move(other.name_);
        this->workDirectory_ = std::move(other.workDirectory_);
        this->args_ = std::move(other.args_);
        this->ReleaseHandles();
        this->handle_ = other.handle_;
        this->stdin_ = other.stdin_;
        this->stdout_ = other.stdout_;
        this->stderr_ = other.stderr_;
        other.handle_ = invalidHandle;
        other.stdin_ = invalidHandle;
        other.stdout_ = invalidHandle;
        other.stderr_ = invalidHandle;
    }
    return *this;
}

sharpen::Process::~Process() noexcept
{
    this->ReleaseHandles();
}

void sharpen::Process::WinStart()
{
#ifdef SHARPEN_IS_WIN
    std::size_t commandLength{this->name_.size() + 3};
    for (auto begin = this->args_.begin(), end = this->args_.end(); begin != end; ++begin)
    {
        if (!begin->empty())
        {
            commandLength += begin->size() + 3;
        }
    }
    std::string commandArgs;
    commandArgs.resize(commandLength);
    commandArgs[0] = '\"';
    std::memcpy(const_cast<char *>(commandArgs.data() + 1), this->name_.data(), this->name_.size());
    commandArgs[this->name_.size() + 1] = '\"';
    commandArgs[this->name_.size() + 2] = ' ';
    std::size_t offset{this->name_.size() + 3};
    for (auto begin = this->args_.begin(), end = this->args_.end(); begin != end; ++begin)
    {
        if (!begin->empty())
        {
            char *data{const_cast<char *>(commandArgs.data())};
            data[offset] = '\"';
            offset += 1;
            std::memcpy(data + offset, begin->data(), begin->size());
            offset += begin->size();
            data[offset] = '\"';
            offset += 1;
            data[offset] = ' ';
            offset += 1;
        }
    }
    commandArgs.pop_back();
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    std::memset(&si, 0, sizeof(si));
    std::memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    BOOL inheritHandles{FALSE};
    if (this->stdin_ != invalidHandle || this->stdout_ != invalidHandle ||
        this->stderr_ != invalidHandle)
    {
        inheritHandles = TRUE;
        si.dwFlags |= STARTF_USESTDHANDLES;
    }
    const char *workDir{nullptr};
    if (!this->workDirectory_.empty())
    {
        workDir = this->workDirectory_.data();
    }
    if (this->stdin_ != invalidHandle)
    {
        si.hStdInput = this->stdin_;
    }
    else if (inheritHandles)
    {
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
        this->stdin_ = handle;
        si.hStdInput = handle;
    }
    if (this->stdout_ != invalidHandle)
    {
        si.hStdOutput = this->stdout_;
    }
    else if (inheritHandles)
    {
        sharpen::FileHandle handle = ::CreateFileA("CONOUT$",
                                                   FILE_GENERIC_WRITE,
                                                   FILE_SHARE_WRITE,
                                                   nullptr,
                                                   OPEN_EXISTING,
                                                   FILE_FLAG_OVERLAPPED,
                                                   nullptr);
        if (handle == INVALID_HANDLE_VALUE)
        {
            sharpen::ThrowLastError();
        }
        this->stdout_ = handle;
        si.hStdOutput = handle;
    }
    if (this->stderr_ != invalidHandle)
    {
        si.hStdError = this->stderr_;
    }
    else if (inheritHandles)
    {
        sharpen::FileHandle handle = ::CreateFileA("CONOUT$",
                                                   FILE_GENERIC_WRITE,
                                                   FILE_SHARE_WRITE,
                                                   nullptr,
                                                   OPEN_EXISTING,
                                                   FILE_FLAG_OVERLAPPED,
                                                   nullptr);
        if (handle == INVALID_HANDLE_VALUE)
        {
            sharpen::ThrowLastError();
        }
        this->stderr_ = handle;
        si.hStdError = handle;
    }
    char *cmd{const_cast<char *>(commandArgs.c_str())};
    if (!::CreateProcessA(this->name_.c_str(),
                          cmd,
                          nullptr,
                          nullptr,
                          TRUE,
                          CREATE_NEW_CONSOLE,
                          nullptr,
                          workDir,
                          &si,
                          &pi))
    {
        sharpen::ThrowLastError();
    }
    sharpen::CloseFileHandle(pi.hThread);
    this->handle_ = pi.hProcess;
#else
    assert(false && "unreachable");
#endif
}

void sharpen::Process::NixStart()
{
#ifdef SHARPEN_IS_NIX
    std::size_t argvSize{2 + this->args_.size()};
    char **argv = reinterpret_cast<char **>(std::calloc(argvSize, sizeof(char *)));
    if (!argv)
    {
        throw std::bad_alloc{};
    }
    argv[0] = const_cast<char *>(this->name_.c_str());
    argv[argvSize - 1] = nullptr;
    std::size_t index{1};
    for (auto begin = this->args_.begin(), end = this->args_.end(); begin != end; ++begin)
    {
        argv[index] = const_cast<char *>(begin->c_str());
        index += 1;
    }
    sharpen::FileHandle errorPipes[2];
    if (::pipe2(errorPipes, O_CLOEXEC) == -1)
    {
        sharpen::ThrowLastError();
    }
    sharpen::FileHandle readPipe{errorPipes[0]};
    sharpen::FileHandle writePipe{errorPipes[1]};
    pid_t pid{::fork()};
    if (pid == -1)
    {
        sharpen::ThrowLastError();
    }
    if (!pid)
    {
        ::close(readPipe);
        if (this->stdin_ != invalidHandle)
        {
            if (::dup2(this->stdin_, STDIN_FILENO) == -1)
            {
                ::write(writePipe, &errno, sizeof(errno));
                std::terminate();
            }
            ::close(this->stdin_);
            this->stdin_ = STDIN_FILENO;
        }
        if (this->stdout_ != invalidHandle)
        {
            if (::dup2(this->stdout_, STDOUT_FILENO) == -1)
            {
                ::write(writePipe, &errno, sizeof(errno));
                std::terminate();
            }
            ::close(this->stdout_);
            this->stdout_ = STDOUT_FILENO;
        }
        if (this->stderr_ != invalidHandle)
        {
            if (::dup2(this->stderr_, STDERR_FILENO) == -1)
            {
                ::write(writePipe, &errno, sizeof(errno));
                std::terminate();
            }
            ::close(this->stderr_);
            this->stderr_ = STDERR_FILENO;
        }
        if (::execv(this->name_.c_str(), argv) == -1)
        {
            ::write(writePipe, &errno, sizeof(errno));
            std::terminate();
        }
        assert(false && "unreachable");
    }
    this->handle_ = pid;
    ::close(this->stdin_);
    ::close(this->stdout_);
    ::close(this->stderr_);
    this->stdin_ = invalidHandle;
    this->stdout_ = invalidHandle;
    this->stderr_ = invalidHandle;
    ::close(writePipe);
    sharpen::ErrorCode err{0};
    ssize_t sz{::read(readPipe, &err, sizeof(err))};
    while (sz == -1 && errno == EINTR)
    {
        sz = ::read(readPipe, &err, sizeof(err));
    }
    if (sz != 0)
    {
        std::size_t offset{static_cast<std::size_t>(sz)};
        while (offset != sizeof(err))
        {
            char *p{reinterpret_cast<char *>(&err)};
            sz = ::read(readPipe, p + offset, sizeof(err) - offset);
            assert(sz != 0);
            while (sz == -1 && errno == EINTR)
            {
                sz = ::read(readPipe, p + offset, sizeof(err) - offset);
            }
            offset += sz;
        }
        ::close(readPipe);
        sharpen::ThrowSystemError(err);
    }
    ::close(readPipe);
#else
    assert(false && "unreachable");
#endif
}

void sharpen::Process::Start()
{
    assert(this->handle_ == invalidHandle);
    assert(!this->name_.empty());
#ifdef SHARPEN_IS_WIN
    this->WinStart();
#else
    this->NixStart();
#endif
}

std::int32_t sharpen::Process::WinJoin()
{
#ifdef SHARPEN_IS_WIN
    DWORD exitCode{STILL_ACTIVE};
    if (!::GetExitCodeProcess(this->handle_, &exitCode))
    {
        sharpen::ThrowLastError();
    }
    while (exitCode == STILL_ACTIVE)
    {
        sharpen::YieldCycle();
        if (!::GetExitCodeProcess(this->handle_, &exitCode))
        {
            sharpen::ThrowLastError();
        }
    }
    return static_cast<std::int32_t>(exitCode);
#else
    assert(false && "unreachable");
    return 0;
#endif
}

std::int32_t sharpen::Process::NixJoin()
{
#ifdef SHARPEN_IS_NIX
    int status{0};
    pid_t pid{0};
    while (!pid)
    {
        pid = ::waitpid(this->handle_, &status, WNOHANG);
        while (pid == -1 && sharpen::GetLastError() == EINTR)
        {
            pid = ::waitpid(this->handle_, &status, WNOHANG);
        }
        if (!pid)
        {
            sharpen::YieldCycle();
        }
    }
    if (pid == -1)
    {
        sharpen::ThrowLastError();
    }
    status = WEXITSTATUS(status);
    return static_cast<std::int32_t>(status);
#else
    assert(false && "unreachable");
    return 0;
#endif
}

std::int32_t sharpen::Process::Join()
{
    assert(this->handle_ != invalidHandle);
#ifdef SHARPEN_IS_WIN
    return this->WinJoin();
#else
    return this->NixJoin();
#endif
}

void sharpen::Process::WinKill()
{
#ifdef SHARPEN_IS_WIN
    if (!::TerminateProcess(this->handle_, EXIT_FAILURE))
    {
        sharpen::ThrowLastError();
    }
#else
    assert(false && "unreachable");
#endif
}

void sharpen::Process::NixKill()
{
#ifdef SHARPEN_IS_NIX
    if (::kill(this->handle_, SIGKILL) == -1)
    {
        sharpen::ThrowLastError();
    }
#else
    assert(false && "unreachable");
#endif
}

void sharpen::Process::Kill()
{
    assert(this->handle_ != invalidHandle);
#ifdef SHARPEN_IS_WIN
    this->WinKill();
#else
    this->NixKill();
#endif
}

void sharpen::Process::Suspend()
{
    assert(this->handle_ != invalidHandle);
    std::uint32_t processId{0};
#ifdef SHARPEN_IS_WIN
    processId = ::GetProcessId(this->handle_);
    if (!processId)
    {
        sharpen::ThrowLastError();
    }
#else
    processId = this->handle_;
#endif
    sharpen::SuspendProcess(processId);
}

void sharpen::Process::Resume()
{
    assert(this->handle_ != invalidHandle);
    std::uint32_t processId{0};
#ifdef SHARPEN_IS_WIN
    processId = ::GetProcessId(this->handle_);
    if (!processId)
    {
        sharpen::ThrowLastError();
    }
#else
    processId = this->handle_;
#endif
    sharpen::ResumeProcess(processId);
}

sharpen::OutputPipeChannelPtr sharpen::Process::RedirectStdin()
{
    assert(this->handle_ == invalidHandle);
#ifdef SHARPEN_IS_WIN
    HANDLE readPipe{invalidHandle};
    HANDLE writePipe{invalidHandle};
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;
    if (!::CreatePipeEx(&readPipe, &writePipe, &saAttr, 0, 0, FILE_FLAG_OVERLAPPED))
    {
        sharpen::ThrowLastError();
    }
    this->stdin_ = readPipe;
    sharpen::OutputPipeChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::WinOutputPipeChannel>(writePipe);
    }
    catch (const std::bad_alloc &)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        this->stdin_ = invalidHandle;
        throw;
    }
    return channel;
#else
    sharpen::FileHandle pipes[2];
    if (::pipe(pipes) == -1)
    {
        sharpen::ThrowLastError();
    }
    sharpen::FileHandle readPipe{pipes[0]};
    sharpen::FileHandle writePipe{pipes[1]};
    int flags{::fcntl(writePipe, F_GETFL)};
    if (flags == -1)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        sharpen::ThrowLastError();
    }
    flags |= O_NONBLOCK | O_CLOEXEC;
    if (::fcntl(writePipe, F_SETFL, flags) == -1)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        sharpen::ThrowLastError();
    }
    this->stdin_ = readPipe;
    sharpen::OutputPipeChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::PosixOutputPipeChannel>(writePipe);
    }
    catch (const std::bad_alloc &)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        this->stdin_ = invalidHandle;
        throw;
    }
    return channel;
#endif
}

sharpen::InputPipeChannelPtr sharpen::Process::RedirectStdout()
{
    assert(this->handle_ == invalidHandle);
#ifdef SHARPEN_IS_WIN
    HANDLE readPipe{invalidHandle};
    HANDLE writePipe{invalidHandle};
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;
    if (!::CreatePipeEx(&readPipe, &writePipe, &saAttr, 0, FILE_FLAG_OVERLAPPED, 0))
    {
        sharpen::ThrowLastError();
    }
    this->stdout_ = writePipe;
    sharpen::InputPipeChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::WinInputPipeChannel>(readPipe);
    }
    catch (const std::bad_alloc &)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        this->stdout_ = invalidHandle;
        throw;
    }
    return channel;
#else
    sharpen::FileHandle pipes[2];
    if (::pipe(pipes) == -1)
    {
        sharpen::ThrowLastError();
    }
    sharpen::FileHandle readPipe{pipes[0]};
    sharpen::FileHandle writePipe{pipes[1]};
    int flags{::fcntl(readPipe, F_GETFL)};
    if (flags == -1)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        sharpen::ThrowLastError();
    }
    flags |= O_NONBLOCK | O_CLOEXEC;
    if (::fcntl(readPipe, F_SETFL, flags) == -1)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        sharpen::ThrowLastError();
    }
    this->stdout_ = writePipe;
    sharpen::InputPipeChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::PosixInputPipeChannel>(readPipe);
    }
    catch (const std::bad_alloc &)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        this->stdout_ = invalidHandle;
        throw;
    }
    return channel;
#endif
}

sharpen::InputPipeChannelPtr sharpen::Process::RedirectStderr()
{
    assert(this->handle_ == invalidHandle);
#ifdef SHARPEN_IS_WIN
    HANDLE readPipe{invalidHandle};
    HANDLE writePipe{invalidHandle};
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;
    if (!::CreatePipeEx(&readPipe, &writePipe, &saAttr, 0, FILE_FLAG_OVERLAPPED, 0))
    {
        sharpen::ThrowLastError();
    }
    this->stderr_ = writePipe;
    sharpen::InputPipeChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::WinInputPipeChannel>(readPipe);
    }
    catch (const std::bad_alloc &)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        this->stderr_ = invalidHandle;
        throw;
    }
    return channel;
#else
    sharpen::FileHandle pipes[2];
    if (::pipe(pipes) == -1)
    {
        sharpen::ThrowLastError();
    }
    sharpen::FileHandle readPipe{pipes[0]};
    sharpen::FileHandle writePipe{pipes[1]};
    int flags{::fcntl(readPipe, F_GETFL)};
    if (flags == -1)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        sharpen::ThrowLastError();
    }
    flags |= O_NONBLOCK | O_CLOEXEC;
    if (::fcntl(readPipe, F_SETFL, flags) == -1)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        sharpen::ThrowLastError();
    }
    this->stderr_ = writePipe;
    sharpen::InputPipeChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::PosixInputPipeChannel>(readPipe);
    }
    catch (const std::bad_alloc &)
    {
        sharpen::CloseFileHandle(readPipe);
        sharpen::CloseFileHandle(writePipe);
        this->stderr_ = invalidHandle;
        throw;
    }
    return channel;
#endif
}