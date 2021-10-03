#include <sharpen/MemoryStack.hpp>

#include <cstdlib>
#include <type_traits>
#include <stdexcept>
#include <cstring>

#include <sharpen/SystemMacro.hpp>

sharpen::MemoryStack::MemoryStack()
    :mem_(nullptr)
    ,size_(0)
{}

sharpen::MemoryStack::MemoryStack(void *mem,sharpen::Size size)
    :mem_(mem)
    ,size_(size)
{}

sharpen::MemoryStack::MemoryStack(sharpen::MemoryStack &&other) noexcept
    :mem_(nullptr)
    ,size_(0)
{
    this->Swap(other);
}

sharpen::MemoryStack::~MemoryStack() noexcept
{
    this->Release();
}

sharpen::MemoryStack &sharpen::MemoryStack::operator=(sharpen::MemoryStack &&other) noexcept
{
    if(this == std::addressof(other))
    {
        return *this;
    }
    this->size_ = 0;
    this->mem_ = nullptr;
    this->Swap(other);
    return *this;
}

void sharpen::MemoryStack::Release() noexcept
{
    if (this->mem_)
    {
        std::free(this->mem_);
        this->mem_ = nullptr;
        this->size_ = 0;
    }
}

void *sharpen::MemoryStack::Top() const noexcept
{
    if (this->mem_)
    {
        sharpen::Uintptr p = reinterpret_cast<sharpen::Uintptr>(this->mem_);
        p += this->size_;
        return reinterpret_cast<void*>(p);
    }
    return nullptr;
}

void *sharpen::MemoryStack::Bottom() const noexcept
{
    return this->mem_;
}

sharpen::Size sharpen::MemoryStack::Size() const noexcept
{
    return this->size_;
}

void sharpen::MemoryStack::Swap(sharpen::MemoryStack &other) noexcept
{
    if(&other != this)
    {
        void *mem = this->mem_;
        sharpen::Size size = this->size_;
        this->mem_ = other.mem_;
        this->size_ = other.size_;
        other.mem_ = mem;
        other.size_ = size;
    }
}

sharpen::MemoryStack sharpen::MemoryStack::AllocStack(sharpen::Size size)
{
    if (size == 0)
    {
        return std::move(sharpen::MemoryStack());
    }
    void *mem = ::calloc(size,1);
    if (!mem)
    {
        throw std::bad_alloc();
    }
    sharpen::MemoryStack stack(mem,size);
    return stack;
}

void sharpen::MemoryStack::Extend(sharpen::Size newSize)
{
    if (this->size_ < newSize)
    {
        void *mem = std::calloc(newSize,1);
        if (mem == nullptr)
        {
            throw std::bad_alloc();
        }
        std::memcpy(mem,this->mem_,this->size_);
        this->size_ = newSize;
        this->Release();
        this->mem_ = mem;
    }
}

void sharpen::MemoryStack::ExtendNoSave(sharpen::Size newSize)
{
    if (this->size_ < newSize)
    {
        void *mem = std::calloc(newSize,1);
        if (mem == nullptr)
        {
            throw std::bad_alloc();
        }
        this->size_ = newSize;
        this->Release();
        this->mem_ = mem;
    }
}

void sharpen::MemoryStack::Clean() noexcept
{
    std::memset(this->mem_,0,this->size_);
}

bool sharpen::MemoryStack::Validate() const noexcept
{
    return this->mem_;
}