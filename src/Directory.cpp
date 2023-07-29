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

#include <cassert>
#include <cstring>
#include <memory>
#include <vector>


#ifdef SHARPEN_IS_WIN
static HANDLE invalidHandle{INVALID_HANDLE_VALUE};
#else
static constexpr DIR *invalidHandle{nullptr};
#endif

bool sharpen::Directory::CheckName(const std::string &name) noexcept {
    for (auto begin = name.begin(), end = name.end(); begin != end; ++begin) {
        if (*begin == '*' || *begin == '?' || *begin == '<' || *begin == '>' || *begin == '#' ||
            *begin == '|') {
            return false;
        }
    }
    return true;
}

sharpen::Directory::Directory(std::string name)
    : name_()
    , handle_(invalidHandle) {
    if (!CheckName(name)) {
        ThrowSystemError(sharpen::ErrorFileNotFound);
    }
    this->name_ = std::move(name);
#ifdef SHARPEN_IS_WIN
    for (auto begin = this->name_.begin(), end = this->name_.end(); begin != end; ++begin) {
        if (*begin == '/') {
            *begin = '\\';
        }
    }
#endif
}

sharpen::Directory::Directory(Self &&other) noexcept
    : name_(std::move(other.name_))
    , handle_(other.handle_) {
    other.handle_ = invalidHandle;
}

sharpen::Directory::~Directory() noexcept {
    this->Close();
}

void sharpen::Directory::Close() noexcept {
    if (this->handle_ != invalidHandle) {
#ifdef SHARPEN_IS_WIN
        ::FindClose(this->handle_);
#else
        ::closedir(reinterpret_cast<DIR *>(this->handle_));
#endif
        this->handle_ = invalidHandle;
    }
}

sharpen::Directory &sharpen::Directory::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->name_ = std::move(other.name_);
        this->Close();
        this->handle_ = other.handle_;
        other.handle_ = invalidHandle;
    }
    return *this;
}

bool sharpen::Directory::Exist() const {
    if (this->name_.empty()) {
        return false;
    }
    return sharpen::ExistDirectory(this->name_.c_str());
}

sharpen::Dentry sharpen::Directory::InternalGetNextEntry() const {
    if (!this->Exist()) {
        sharpen::ThrowSystemError(sharpen::ErrorFileNotFound);
    }
    sharpen::FileEntryType type{sharpen::FileEntryType::File};
    std::string name;
#ifdef SHARPEN_IS_WIN
    WIN32_FIND_DATA findData;
    bool found{false};
    if (this->handle_ == invalidHandle) {
        char back{this->name_.back()};
        std::size_t needSize{this->name_.size() + 1};
        if (!sharpen::IsPathSeparator(back)) {
            needSize += 1;
        }
        if (needSize > sharpen::GetMaxPath()) {
            sharpen::ThrowSystemError(sharpen::ErrorNameTooLong);
        }
        thread_local char path[sharpen::GetMaxPath() + 1] = {0};
        std::memcpy(path, this->name_.c_str(), this->name_.size());
        std::size_t index{this->name_.size()};
        if (back != '\\') {
            path[index++] = '\\';
        }
        path[index] = '*';
        sharpen::FileHandle handle{::FindFirstFileA(path, &findData)};
        if (handle == invalidHandle) {
            sharpen::ThrowLastError();
        }
        this->handle_ = handle;
        found = true;
    } else {
        found = ::FindNextFileA(this->handle_, &findData) == TRUE;
    }
    if (found) {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            type = sharpen::FileEntryType::Directory;
        } else if (findData.dwFileAttributes &FILE_ATTRIBUTE_REPARSE_POINT) {
            type = sharpen::FileEntryType::SymbolicLink;
        }
        name.assign(findData.cFileName);
    }
#else
    if (this->handle_ == invalidHandle) {
        DIR *dir{::opendir(this->name_.c_str())};
        if (!dir) {
            sharpen::ThrowLastError();
        }
        this->handle_ = dir;
    }
    dirent *dentry{::readdir(reinterpret_cast<DIR *>(this->handle_))};
    if (dentry != nullptr) {
        if (dentry->d_type == DT_DIR) {
            type = sharpen::FileEntryType::Directory;
        } else if(dentry->d_type == DT_FIFO) {
            type = sharpen::FileEntryType::Pipe;
        } else if(dentry->d_type == DT_LNK) {
            type = sharpen::FileEntryType::SymbolicLink;
        } else if(dentry->d_type == DT_SOCK) {
            type = sharpen::FileEntryType::UnixSocket;
        } else if(dentry->d_type == DT_BLK) {
            type = sharpen::FileEntryType::BlockDevice;
        } else if(dentry->d_type == DT_CHR) {
            type = sharpen::FileEntryType::CharDevice;
        }
        name.assign(dentry->d_name);
    } else if (errno) {
        sharpen::ThrowLastError();
    }
#endif
    return sharpen::Dentry{type, std::move(name)};
}

sharpen::Dentry sharpen::Directory::GetNextEntry() const {
    sharpen::Dentry entry{this->InternalGetNextEntry()};
    while (entry.Valid()) {
        if (entry.Name() == ".." || entry.Name() == ".") {
            entry = this->InternalGetNextEntry();
            continue;
        }
#ifdef SHARPEN_IS_WIN
        if (this->name_.size() + entry.Name().size() + this->name_.back() != '\\'
                ? 1
                : 0 <= sharpen::GetMaxPath()) {
            thread_local char path[sharpen::GetMaxPath() + 1] = {0};
            std::memcpy(path, this->name_.c_str(), this->name_.size());
            std::size_t index{this->name_.size()};
            if (this->name_.back() != '\\') {
                path[index++] = '\\';
            }
            std::memcpy(path + index, entry.Name().data(), entry.Name().size());
            switch (entry.GetType()) {
            case sharpen::FileEntryType::File: {
                if (!sharpen::ExistFile(path)) {
                    entry = this->InternalGetNextEntry();
                    continue;
                }
            } break;
            case sharpen::FileEntryType::Directory: {
                if (!sharpen::ExistDirectory(path)) {
                    entry = this->InternalGetNextEntry();
                    continue;
                }
            } break;
            default: {
                // do nothing
            } break;
            }
        }
#endif
        break;
    }
    return entry;
}

sharpen::RmdirResult sharpen::Directory::Remove() {
    if (!this->name_.empty()) {
        this->Close();
        return sharpen::DeleteDirectory(this->name_.c_str());
    }
    return sharpen::RmdirResult::NotExists;
}

sharpen::RmdirResult sharpen::Directory::RemoveAll() {
    if (!this->Exist()) {
        return sharpen::RmdirResult::NotExists;
    }
    std::vector<std::unique_ptr<sharpen::Directory>> dirs;
    std::unique_ptr<sharpen::Directory> dirPtr{new (std::nothrow) sharpen::Directory{this->name_}};
    if (!dirPtr) {
        throw std::bad_alloc{};
    }
    dirs.emplace_back(std::move(dirPtr));
    for (std::size_t i = 0; i != dirs.size(); ++i) {
        auto &dir{*dirs[i]};
        for (auto begin = dir.Begin(), end = dir.End(); begin != end; ++begin) {
            if (begin->Valid()) {
                // compute buf size
                std::size_t size{dir.Path().size()};
                if (!sharpen::IsPathSeparator(dir.Path().back())) {
                    size += 1;
                }
                size += begin->Name().size();
                // copy to name
                std::string name;
                name.resize(size, '\0');
                std::memcpy(const_cast<char *>(name.data()), dir.Path().data(), dir.Path().size());
                std::size_t offset{dir.Path().size()};
                if (!sharpen::IsPathSeparator(dir.Path().back())) {
                    name[offset] = '/';
                    offset += 1;
                }
                std::memcpy(const_cast<char *>(name.data() + offset),
                            begin->Name().data(),
                            begin->Name().size());
                switch (begin->GetType()) {
                case sharpen::FileEntryType::File: {
                    sharpen::RemoveFile(name.c_str());
                } break;
                case sharpen::FileEntryType::Directory: {
                    dirPtr = std::unique_ptr<sharpen::Directory>{
                        new (std::nothrow) sharpen::Directory{std::move(name)}};
                    if (!dirPtr) {
                        throw std::bad_alloc{};
                    }
                    dirs.emplace_back(std::move(dirPtr));
                } break;
                case sharpen::FileEntryType::SymbolicLink: {
                    sharpen::RemoveFile(name.c_str());
                }
                default: {
                    // do nothing
                } break;
                }
            }
        }
    }
    for (auto begin = dirs.begin() + 1, end = dirs.end(); begin != end; ++begin) {
        auto &dir{*begin};
        dir->Remove();
    }
    dirs.clear();
    return this->Remove();
}

sharpen::MkdirResult sharpen::Directory::Create() {
    assert(!this->name_.empty());
    return sharpen::MakeDirectory(this->name_.c_str());
}