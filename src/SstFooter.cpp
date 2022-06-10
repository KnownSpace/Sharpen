#include <sharpen/SstFooter.hpp>

#include <stdexcept>
#include <cassert>

void sharpen::SstFooter::LoadFrom(const char *data,std::size_t size)
{
    if(size < sizeof(*this))
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::memcpy(&this->indexBlock_,data,sizeof(this->indexBlock_));
    std::memcpy(&this->metaIndexBlock_,data + sizeof(this->indexBlock_),sizeof(this->metaIndexBlock_));
}

void sharpen::SstFooter::LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

void sharpen::SstFooter::UnsafeStoreTo(char *data) const noexcept
{
    std::memcpy(data,&this->indexBlock_,sizeof(this->indexBlock_));
    std::memcpy(data + sizeof(this->indexBlock_),&this->metaIndexBlock_,sizeof(this->metaIndexBlock_));
}

void sharpen::SstFooter::StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const
{
    std::size_t size = buf.GetSize() - offset;
    if(size < sizeof(*this))
    {
        buf.Extend(sizeof(*this) - size);
    }
    this->UnsafeStoreTo(buf.Data() + offset);
}

void sharpen::SstFooter::StoreTo(char *data,std::size_t size) const
{
    if(size < sizeof(*this))
    {
        throw std::invalid_argument("buffer too small");
    }
    this->UnsafeStoreTo(data);
}