#include <sharpen/FileOps.hpp>

#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <cassert>
#include <cstring>

bool sharpen::AccessFile(const char *name, sharpen::FileAccessMethod model) {
    int mod = 0;
#ifdef SHARPEN_IS_WIN
    switch (model) {
    case sharpen::FileAccessMethod::Read:
        mod = 4;
        break;
    case sharpen::FileAccessMethod::Write:
        mod = 2;
        break;
    case sharpen::FileAccessMethod::All:
        mod = 6;
        break;
    default:
        break;
    }
    return _access_s(name, mod) == 0;
#else
    switch (model) {
    case sharpen::FileAccessMethod::Read:
        mod = R_OK;
        break;
    case sharpen::FileAccessMethod::Write:
        mod = W_OK;
        break;
    case sharpen::FileAccessMethod::All:
        mod = R_OK | W_OK;
        break;
    default:
        break;
    }
    return ::access(name, mod) == 0;
#endif
}

bool sharpen::ExistFile(const char *name) {
#ifdef SHARPEN_IS_WIN
    return ::_access_s(name, 0) == 0;
#else
    return ::access(name, F_OK) == 0;
#endif
}

void sharpen::RenameFile(const char *oldName, const char *newName) {
#ifdef SHARPEN_IS_WIN
    if (::MoveFileExA(oldName, newName, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) ==
        FALSE) {
        sharpen::ThrowLastError();
    }
#else
    if (::rename(oldName, newName) == -1) {
        sharpen::ThrowLastError();
    }
#endif
}

void sharpen::RemoveFile(const char *name) {
#ifdef SHARPEN_IS_WIN
    if (::DeleteFileA(name) == FALSE) {
        sharpen::ThrowLastError();
    }
#else
    if (::remove(name) == -1) {
        sharpen::ThrowLastError();
    }
#endif
}

bool sharpen::GetCurrentWorkDirectory(char *pathBuf, std::size_t size) noexcept {
    if (!pathBuf || !size) {
        return false;
    }
#ifdef SHARPEN_IS_WIN
    return ::GetCurrentDirectoryA(static_cast<DWORD>(size), pathBuf) != 0;
#else
    return ::getcwd(pathBuf, size) != nullptr;
#endif
}

void sharpen::SetCurrentWorkDirectory(const char *path) {
    assert(path);
#ifdef SHARPEN_IS_WIN
    if (::SetCurrentDirectoryA(path) == FALSE) {
        sharpen::ThrowLastError();
    }
#else
    if (::chdir(path) == -1) {
        sharpen::ThrowLastError();
    }
#endif
}

void sharpen::ResolvePath(const char *currentPath,
                          std::size_t currentPathSize,
                          const char *path,
                          std::size_t pathSize,
                          char *resolvedPath,
                          std::size_t resolvedPathSize) {
    if (resolvedPathSize < currentPathSize + pathSize) {
        throw std::invalid_argument("resolved path size too small");
    }
    if (currentPathSize) {
        if (currentPath[currentPathSize - 1] != '/') {
            throw std::invalid_argument("invalid current path");
        }
        std::memcpy(resolvedPath, currentPath, currentPathSize);
    }
    char *resolvedBegin = resolvedPath;
    char *resolved = resolvedPath + currentPathSize;
    char *resolvedEnd = resolvedPath + resolvedPathSize;
    // state machine
    std::size_t pointNumber{0};
    std::size_t separatorNumber{1};
    for (const char *begin = path, *end = path + pathSize; begin != end; ++begin) {
        assert(resolved < resolvedEnd);
        if (*begin == '.') {
            if (separatorNumber) {
                pointNumber += 1;
            } else {
                *resolved++ = *begin;
            }
        } else if (sharpen::IsPathSeparator(*begin)) {
            if (separatorNumber) {
                // current /(.)[/] /[/]
                // up-level /(..)[/]
                if (pointNumber > 1) {
                    // [...](/../)
                    resolved -= 2;
                    // find up-level
                    char *pos = resolved;
                    for (; pos != resolvedBegin - 1; --pos) {
                        if (sharpen::IsPathSeparator(*pos)) {
                            resolved = pos + 1;
                            break;
                        }
                    }
                    if (pos == resolvedBegin - 1) {
                        throw std::invalid_argument("invalid path");
                    }
                    resolved = pos + 1;
                }
                pointNumber = 0;
                continue;
            }
            separatorNumber += 1;
            *resolved++ = *begin;
        } else {
            separatorNumber = 0;
            for (std::size_t i = 0; i < pointNumber; ++i) {
                *resolved++ = '.';
            }
            pointNumber = 0;
            *resolved++ = *begin;
        }
    }
    if (separatorNumber) {
        if (pointNumber > 1) {
            resolved -= 2;
            // find up-level
            char *pos = resolved;
            for (; pos != resolvedBegin - 1; --pos) {
                if (sharpen::IsPathSeparator(*pos)) {
                    break;
                }
            }
            if (pos == resolvedBegin - 1) {
                throw std::invalid_argument("invalid path");
            }
            resolved = pos + 1;
        }
    }
    std::memset(resolved, 0, resolvedEnd - resolved);
}

void sharpen::MakeDirectory(const char *name) {
#ifdef SHARPEN_IS_WIN
    if (::CreateDirectoryA(name, nullptr) == FALSE &&
        sharpen::GetLastError() != ERROR_ALREADY_EXISTS) {
        sharpen::ThrowLastError();
    }
#else
    if (::mkdir(name, S_IRWXU) == -1 && sharpen::GetLastError() != EEXIST) {
        sharpen::ThrowLastError();
    }
#endif
}

void sharpen::DeleteDirectory(const char *name) {
#ifdef SHARPEN_IS_WIN
    if (::RemoveDirectoryA(name) == FALSE && sharpen::GetLastError() != ERROR_FILE_NOT_FOUND) {
        sharpen::ThrowLastError();
    }
#else
    if (::rmdir(name) == -1 && sharpen::GetLastError() != ENOENT) {
        sharpen::ThrowLastError();
    }
#endif
}
