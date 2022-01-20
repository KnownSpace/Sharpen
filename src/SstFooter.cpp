#include <sharpen/SstFooter.hpp>

#include <stdexcept>
#include <cassert>

void sharpen::SstFooter::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < sizeof(*this))
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::memcpy(&this->indexBlock_,data,sizeof(this->indexBlock_));
    std::memcpy(&this->metaIndexBlock_,data + sizeof(this->indexBlock_),sizeof(this->metaIndexBlock_));
}

void sharpen::SstFooter::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() > offset);
    this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

void sharpen::SstFooter::InternalStoreTo(char *data) const noexcept
{
    std::memcpy(data,&this->indexBlock_,sizeof(this->indexBlock_));
    std::memcpy(data + sizeof(this->indexBlock_),&this->metaIndexBlock_,sizeof(this->metaIndexBlock_));
}

void sharpen::SstFooter::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    sharpen::Size size = buf.GetSize() - offset;
    if(size < sizeof(*this))
    {
        buf.Extend(sizeof(*this) - size);
    }
    this->InternalStoreTo(buf.Data() + offset);
}

void sharpen::SstFooter::StoreTo(char *data,sharpen::Size size) const
{
    if(size < sizeof(*this))
    {
        throw std::invalid_argument("buffer too small");
    }
    this->InternalStoreTo(data);
}