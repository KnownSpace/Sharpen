#include <sharpen/LevelComponent.hpp>

#include <sharpen/Varint.hpp>
#include <sharpen/IntOps.hpp>

sharpen::LevelComponent::LevelComponent()
    :Self(Self::defaultReserveSize)
{}

sharpen::LevelComponent::LevelComponent(std::size_t reserveSize)
    :views_()
{
    this->views_.reserve(reserveSize);
}

std::size_t sharpen::LevelComponent::LoadFrom(const char *data,std::size_t size)
{
    if(size < 1)
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::size_t offset{0};
    //load view count
    sharpen::Varuint64 builder{data,size};
    std::size_t viewCount{sharpen::IntCast<std::size_t>(builder.Get())};
    offset += builder.ComputeSize();
    this->views_.reserve(viewCount);
    //load views
    for (std::size_t i = 0; i != viewCount; ++i)
    {
        builder.Set(data + offset,size - offset);
        this->views_.emplace_back(builder.Get());
        offset += builder.ComputeSize();   
    }
    return offset;
}

std::size_t sharpen::LevelComponent::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

std::size_t sharpen::LevelComponent::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    //store view count
    sharpen::Varuint64 builder{this->views_.size()};
    std::size_t size{builder.ComputeSize()};
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

std::size_t sharpen::LevelComponent::ComputeSize() const noexcept
{
    std::size_t offset{0};
    //store view count
    sharpen::Varuint64 builder{this->views_.size()};
    std::size_t size{builder.ComputeSize()};
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

std::size_t sharpen::LevelComponent::StoreTo(char *data,std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if (size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::LevelComponent::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}

void sharpen::LevelComponent::Put(std::uint64_t id)
{
    this->views_.emplace_back(id);
}

void sharpen::LevelComponent::Delete(std::uint64_t id)
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