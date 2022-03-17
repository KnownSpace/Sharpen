#include <sharpen/LevelComponent.hpp>

#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::LevelComponent::LevelComponent()
    :Self(Self::defaultReserveSize)
{}

sharpen::LevelComponent::LevelComponent(sharpen::Size reserveSize)
    :views_()
{
    this->views_.reserve(reserveSize);
}

sharpen::Size sharpen::LevelComponent::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < 1)
    {
        throw std::invalid_argument("invalid buffer");
    }
    sharpen::Size offset{0};
    //load view count
    sharpen::Varuint64 builder{data,size};
    sharpen::Size viewCount{sharpen::IntCast<sharpen::Size>(builder.Get())};
    offset += builder.ComputeSize();
    this->views_.reserve(viewCount);
    //load views
    for (sharpen::Size i = 0; i != viewCount; ++i)
    {
        builder.Set(data + offset,size - offset);
        this->views_.emplace_back(builder.Get());
        offset += builder.ComputeSize();   
    }
    return offset;
}

sharpen::Size sharpen::LevelComponent::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::LevelComponent::UnsafeStoreTo(char *data) const noexcept
{
    sharpen::Size offset{0};
    //store view count
    sharpen::Varuint64 builder{this->views_.size()};
    sharpen::Size size{builder.ComputeSize()};
    std::memcpy(data,builder.Data(),size);
    offset += size;
    //store views
    for (auto begin = this->views_.begin(),end = this->views_.end(); begin != end; ++begin)
    {
        builder.Set(*begin);   
        size = builder.ComputeSize();
        std::memcpy(data + offset,builder.Data(),size);
        offset += size;
    }
    return offset;
}

sharpen::Size sharpen::LevelComponent::ComputeSize() const noexcept
{
    sharpen::Size offset{0};
    //store view count
    sharpen::Varuint64 builder{this->views_.size()};
    sharpen::Size size{builder.ComputeSize()};
    offset += size;
    //store views
    for (auto begin = this->views_.begin(),end = this->views_.end(); begin != end; ++begin)
    {
        builder.Set(*begin);   
        size = builder.ComputeSize();
        offset += size;
    }
    return offset;
}

sharpen::Size sharpen::LevelComponent::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if (size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::LevelComponent::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size needSize{this->ComputeSize()};
    sharpen::Size size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

void sharpen::LevelComponent::Put(sharpen::Uint64 id)
{
    this->views_.emplace_back(id);
}

void sharpen::LevelComponent::Delete(sharpen::Uint64 id)
{
    for (auto begin = this->Begin(),end = this->End(); begin != end; ++begin)
    {
        if(*begin == id)
        {
            this->views_.erase(begin);
            return;
        }   
    }
}