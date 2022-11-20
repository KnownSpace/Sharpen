#include <sharpen/MemoryPage.hpp>

sharpen::MemoryPage::MemoryPage() noexcept
    :data_(nullptr)
    ,pageCount_(0)
{}

sharpen::MemoryPage::MemoryPage(std::size_t pageCount)
    :data_(nullptr)
    ,pageCount_(0)
{
    if(pageCount)
    {
        char *data = reinterpret_cast<char*>(sharpen::AlignedAllocPages(pageCount));
        if(!data)
        {
            throw std::bad_alloc{};
        }
        this->data_ = data;
        this->pageCount_ = pageCount;
    }
}

sharpen::MemoryPage::MemoryPage(const Self &other)
    :data_(nullptr)
    ,pageCount_(0)
{
    if(other.pageCount_)
    {
        char *data = reinterpret_cast<char*>(sharpen::AlignedAllocPages(other.pageCount_));
        if(!data)
        {
            throw std::bad_alloc{};
        }
        std::memcpy(data,other.data_,other.GetSize());
        this->data_= data;
        this->pageCount_ = other.pageCount_;
    }
}

sharpen::MemoryPage::MemoryPage(Self &&other) noexcept
    :data_(other.data_)
    ,pageCount_(other.pageCount_)
{
    other.data_ = nullptr;
    other.pageCount_ = 0;
}

sharpen::MemoryPage &sharpen::MemoryPage::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->Free();
        this->data_ = other.data_;
        this->pageCount_ = other.pageCount_;
        other.data_ = nullptr;
        other.pageCount_ = 0;
    }
    return *this;
}

void sharpen::MemoryPage::Free() noexcept
{
    if(this->data_)
    {
        sharpen::AlignedFree(this->data_);
        this->data_ = nullptr;
        this->pageCount_ = 0;
    }
}

sharpen::MemoryPage::~MemoryPage() noexcept
{
    this->Free();
}

sharpen::ByteSlice sharpen::MemoryPage::GetSlice(std::size_t index,std::size_t size) const
{
    if(index + size > this->GetSize())
    {
        throw std::out_of_range{"out of range"};
    }
    return sharpen::ByteSlice{this->data_ + index,size};
}