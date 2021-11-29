#include <sharpen/FileMemory.hpp>

#include <cassert>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_IS_WIN
sharpen::FileMemory::FileMemory(sharpen::FileHandle file,void *address,sharpen::Size size) noexcept
    :file_(file)
    ,address_(address)
    ,size_(size)
{}
#else
sharpen::FileMemory::FileMemory(void *address,sharpen::Size size) noexcept
    :address_(address)
    ,size_(size)
{}
#endif

sharpen::FileMemory::FileMemory(Self &&other) noexcept
    :address_(other.address_)
    ,size_(other.size_)
#ifdef SHARPEN_IS_WIN
    ,file_(other.file_)
#endif
{
    other.address_ = nullptr;
    other.size_ = 0;
#ifdef SHARPEN_IS_WIN
    other.file_ =INVALID_HANDLE_VALUE;
#endif
}

sharpen::FileMemory &sharpen::FileMemory::operator=(Self &&other) noexcept
{
    this->address_ = other.address_;
    this->size_ = other.size_;
#ifdef SHARPEN_IS_WIN
    this->file_ = other.file_;
#endif
    other.address_ = nullptr;
    other.size_ = 0;
#ifdef SHARPEN_IS_WIN
    other.file_ = INVALID_HANDLE_VALUE;
#endif
    return *this;
}

sharpen::FileMemory::~FileMemory () noexcept
{
    if(this->address_)
    {
        assert(this->size_);
#ifdef SHARPEN_IS_WIN
        ::UnmapViewOfFile(this->address_);
#else
        ::munmap(this->address_,this->size_);
#endif
    }
}

void *sharpen::FileMemory::Get() const noexcept
{
    return this->address_;
}

void sharpen::FileMemory::Flush() const
{
    if(this->address_)
    {
        assert(this->size_);
#ifdef SHARPEN_IS_WIN
        if(::FlushViewOfFile(this->address_,this->size_) == FALSE)
        {
            sharpen::ThrowLastError();
        }
#else
        if(::msync(this->address_,this->size_,MS_ASYNC) == -1)
        {
            sharpen::ThrowLastError();
        }
#endif
    }
}

void sharpen::FileMemory::FlushSync() const
{
    if(this->address_)
    {
        assert(this->size_);
#ifdef SHARPEN_IS_WIN
        if(::FlushViewOfFile(this->address_,this->size_) == FALSE)
        {
            sharpen::ThrowLastError();
        }        
        if(::FlushFileBuffers(this->file_) == FALSE)
        {
            sharpen::ThrowLastError();
        }
#else
        if(::msync(this->address_,this->size_,MS_SYNC) == -1)
        {
            sharpen::ThrowLastError();
        }
#endif
    }
}