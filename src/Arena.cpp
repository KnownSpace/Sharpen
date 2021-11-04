#include <sharpen/Arena.hpp>

#include <mutex>

sharpen::Arena::Arena(sharpen::Size blockSize)
    :smallBlocks_(sharpen::Arena::AllocSmallBlock(blockSize))
    ,largeBlocks_(nullptr)
    ,lock_()
{}

sharpen::Arena::SmallBlock *sharpen::Arena::AllocSmallBlock(sharpen::Size size) noexcept
{
    if (size < sizeof(SmallBlock))
    {
        return nullptr;
    }
    SmallBlock *sb = reinterpret_cast<SmallBlock*>(std::malloc(size));
    if (!sb)
    {
        return nullptr;
    }
    char *byte = reinterpret_cast<char*>(sb);
    sb->next_ = nullptr;
    // sb->curr_ = byte + sizeof(*sb);
    new (&sb->curr_) std::atomic<char*>(byte + sizeof(*sb));
    sb->end_ = byte + size;
    return sb;
}

sharpen::Arena::~Arena() noexcept
{
    SmallBlock *sb = this->smallBlocks_;
    while (sb != nullptr)
    {
        SmallBlock *tmp = sb->next_;
        sharpen::Arena::Destruct(&sb->curr_);
        std::free(sb);
        sb = tmp;   
    }
    LargeBlock *lb = this->largeBlocks_;
    while (lb != nullptr)
    {
        LargeBlock *tmp = lb->next_;
        std::free(lb);
        lb = tmp;
    }
}

void *sharpen::Arena::AllocSmall(sharpen::Size size) noexcept
{
    SmallBlock *sb = this->smallBlocks_;
    char *curr = sb->curr_.load();
    do
    {
        sharpen::Size remain = sb->end_ - curr;
        if (remain < size)
        {
            curr = nullptr;
            break;
        }
    } while (!sb->curr_.compare_exchange_weak(curr,curr + size));
    if (curr)
    {
        return curr;
    }
    {
        std::unique_lock<sharpen::SpinLock> lock(this->lock_);
        sb = sharpen::Arena::AllocSmallBlock(maxAlloc_ + sizeof(*sb));
        if (!sb)
        {
            return nullptr;
        }
        sb->next_ = this->smallBlocks_;
        curr = sb->curr_;
        sb->curr_ += size;
        this->smallBlocks_ = sb;
    }
    return curr;
}

void *sharpen::Arena::AllocLarge(sharpen::Size size) noexcept
{
    LargeBlock *lb = reinterpret_cast<LargeBlock*>(std::malloc(sizeof(*lb) + size));
    if (!lb)
    {
        return nullptr;
    }
    LargeBlock *first = this->largeBlocks_;
    do
    {
        lb->next_ = first;
    } while (this->largeBlocks_.compare_exchange_weak(first,lb));
    char *byte = reinterpret_cast<char*>(lb);
    byte += sizeof(*lb);
    return byte;
}