#include <sharpen/Directory.hpp>

#include <sharpen/FileOps.hpp>
#include <sharpen/SystemError.hpp>


#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <cstring>

#ifdef SHARPEN_IS_WIN
static HANDLE invalidHandle{INVALID_HANDLE_VALUE};
#else
static constexpr DIR *invalidHandle{nullptr};
#endif

bool sharpen::Directory::CheckName(const std::string &name) noexcept
{
    for (auto begin = name.begin(), end = name.end(); begin != end; ++begin)
    {
        if (*begin == '*' || *begin == '?' || *begin == '<' || *begin == '>')
        {
            return false;
        }
    }
    return true;
}

sharpen::Directory::Directory(std::string name)
    : name_()
    , handle_(invalidHandle)
{
    if (CheckName(name))
    {
        this->name_ = std::move(name);
    }
}

sharpen::Directory::Directory(Self &&other) noexcept
    : name_(std::move(other.name_))
    , handle_(other.handle_)
{
    other.handle_ = invalidHandle;
}

sharpen::Directory::~Directory() noexcept
{
    this->Close();
}

void sharpen::Directory::Close() noexcept
{
    if (this->handle_ != invalidHandle)
    {
#ifdef SHARPEN_IS_WIN
        ::FindClose(this->handle_);
#else
        ::closedir(reinterpret_cast<DIR *>(this->handle_));
#endif
        this->handle_ = invalidHandle;
    }
}

sharpen::Directory &sharpen::Directory::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->name_ = std::move(other.name_);
        this->Close();
        this->handle_ = other.handle_;
        other.handle_ = invalidHandle;
    }
    return *this;
}

bool sharpen::Directory::Exist() const
{
    if (this->name_.empty())
    {
        return false;
    }
    return sharpen::ExistDirectory(this->name_.c_str());
}

sharpen::Dentry sharpen::Directory::GetNextEntry() const
{
    if (!this->Exist())
    {
        sharpen::ThrowSystemError(sharpen::ErrorFileNotFound);
    }
    sharpen::DentryType type{sharpen::DentryType::File};
    std::string name;
#ifdef SHARPEN_IS_WIN
    char back{this->name_.back()};
    std::size_t needSize{this->name_.size() + 1};
    if (back != '/' || back != '\\')
    {
        needSize += 1;
    }
    if (needSize > sharpen::GetMaxPath())
    {
        sharpen::ThrowSystemError(sharpen::ErrorNameTooLong);
    }
    WIN32_FIND_DATA findData;
    char path[sharpen::GetMaxPath() + 1] = {0};
    std::memcpy(path, this->name_.c_str(), this->name_.size());
    std::size_t index{this->name_.size()};
    if (back != '\\')
    {
        if (back != '/')
        {
            path[index++] = '\\';
        }
        else
        {
            path[index - 1] = '\\';
        }
    }
    path[index] = '*';
    bool found{false};
    if (this->handle_ == invalidHandle)
    {
        sharpen::FileHandle handle{::FindFirstFileA(path, &findData)};
        if (handle == invalidHandle)
        {
            sharpen::ThrowLastError();
        }
        this->handle_ = handle;
        found = true;
    }
    else
    {
        found = ::FindNextFileA(this->handle_, &findData) == TRUE;
    }
    if (found)
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            type = sharpen::DentryType::Directory;
        }
        name.assign(findData.cFileName);
    }
#else
    if (this->handle_ == invalidHandle)
    {
        DIR *dir{::opendir(this->name_.c_str())};
        if (!dir)
        {
            sharpen::ThrowLastError();
        }
        this->handle_ = dir;
    }
    dirent *dentry{::readdir(reinterpret_cast<DIR*>(this->handle_))};
    if (dentry != nullptr)
    {
        if (dentry->d_type == DT_DIR || dentry->d_type == DT_REG)
        {
            if (dentry->d_type == DT_DIR)
            {
                type = sharpen::DentryType::Directory;
            }
            name.assign(dentry->d_name);
        }
    }
    else if (errno)
    {
        sharpen::ThrowLastError();
    }
#endif
    return sharpen::Dentry{type, std::move(name)};
}

sharpen::Dentry sharpen::Directory::GetNextEntry(bool excludeUpper) const
{
    if(excludeUpper)
    {
        sharpen::Dentry entry{this->GetNextEntry()};
        if(entry.Valid() && entry.Name() == "..")
        {
            entry = this->GetNextEntry();
        }
        return entry;
    }
    return this->GetNextEntry();
}