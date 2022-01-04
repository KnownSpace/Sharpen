#include <sharpen/SstFooter.hpp>

#include <stdexcept>

void sharpen::SstFooter::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    sharpen::Size size = buf.GetSize() - offset;
    if(size < sizeof(*this))
    {
        throw std::invalid_argument("invalid buffer");
    }
    std::memcpy(&this->indexBlock_,buf.Data() + offset,sizeof(this->indexBlock_));
    std::memcpy(&this->metaIndexBlock_,buf.Data() + offset + sizeof(this->indexBlock_),sizeof(this->metaIndexBlock_));
}

void sharpen::SstFooter::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    sharpen::Size size = buf.GetSize() - offset;
    if(size < sizeof(*this))
    {
        buf.Extend(sizeof(*this) - size);
    }
    std::memcpy(buf.Data() + offset,&this->indexBlock_,sizeof(this->indexBlock_));
    std::memcpy(buf.Data() + offset + sizeof(this->indexBlock_),&this->metaIndexBlock_,sizeof(this->metaIndexBlock_));
}