#include <sharpen/MemoryStack.hpp>

#include <stdexcept>

sharpen::MemoryStack::MemoryStack() noexcept
    : mem_(nullptr)
    , size_(0)
{
}

sharpen::MemoryStack::MemoryStack(void *mem, std::size_t size) noexcept
    : mem_(mem)
    , size_(size)
{
}

sharpen::MemoryStack::MemoryStack(sharpen::MemoryStack &&other) noexcept
    : mem_(nullptr)
    , size_(0)
{
    std::swap(other.mem_, this->mem_);
    std::swap(other.size_, this->size_);
}

sharpen::MemoryStack::~MemoryStack() noexcept
{
    this->Release();
}

sharpen::MemoryStack &sharpen::MemoryStack::operator=(sharpen::MemoryStack &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->Release();
        std::swap(other.mem_, this->mem_);
        std::swap(other.size_, this->size_);
    }
    return *this;
}

void sharpen::MemoryStack::Release() noexcept
{
    if (this->mem_)
    {
        this->Free(this->mem_);
        this->mem_ = nullptr;
        this->size_ = 0;
    }
}

void *sharpen::MemoryStack::Top() const noexcept
{
    if (this->mem_)
    {
        std::uintptr_t p = reinterpret_cast<std::uintptr_t>(this->mem_);
        p += this->size_;
        return reinterpret_cast<void *>(p);
    }
    return nullptr;
}

sharpen::MemoryStack sharpen::MemoryStack::AllocStack(std::size_t size)
{
    if (size == 0)
    {
        return std::move(sharpen::MemoryStack());
    }
    void *mem = Self::Alloc(size);
    if (!mem)
    {
        throw std::bad_alloc();
    }
    sharpen::MemoryStack stack(mem, size);
    return stack;
}

void sharpen::MemoryStack::Extend(std::size_t newSize)
{
    if (this->size_ < newSize)
    {
        void *mem = this->Alloc(newSize);
        if (mem == nullptr)
        {
            throw std::bad_alloc();
        }
        std::memcpy(mem, this->mem_, this->size_);
        this->Release();
        this->size_ = newSize;
        this->mem_ = mem;
    }
}

void sharpen::MemoryStack::ExtendNoSave(std::size_t newSize)
{
    if (this->size_ < newSize)
    {
        void *mem = std::calloc(newSize, 1);
        if (mem == nullptr)
        {
            throw std::bad_alloc();
        }
        this->Release();
        this->size_ = newSize;
        this->mem_ = mem;
    }
}