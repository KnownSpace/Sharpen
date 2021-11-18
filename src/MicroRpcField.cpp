#include <sharpen/MicroRpcField.hpp>

void sharpen::MicroRpcField::InitHeader(sharpen::MicroRpcVariableType type)
{
    this->data_.Reset();
    //init header
    this->data_.PushBack(0);
    sharpen::MicroRpcFieldHeader &header = this->Header();
    header.type_ = static_cast<unsigned char>(type);
    header.sizeSpace_ = 0;
    header.end_ = 0;
}

char *sharpen::MicroRpcField::ComputeDataBody() noexcept
{
    sharpen::Size size = this->Header().sizeSpace_;
    if (size == 7)
    {
        size += 1;
    }
    return this->data_.Data() + size + 1;
}

const char *sharpen::MicroRpcField::ComputeDataBody() const noexcept
{
    sharpen::Size size = this->Header().sizeSpace_;
    if (size == 7)
    {
        size += 1;
    }
    return this->data_.Data() + size + 1;
}

sharpen::Uint64 sharpen::MicroRpcField::GetSize() const
{
    sharpen::Size size = this->Header().sizeSpace_;
    if (size == 0)
    {
        if (this->GetRawSize() == 1)
        {
            return 0;
        }
        return 1;
    }
    if (size == 7)
    {
        size += 1;
    }
    if (size > sizeof(sharpen::Size))
    {
        throw std::length_error("element count overflow");
    }
    sharpen::Size count{0};
    std::memcpy(&count, this->data_.Data() + 1, size);
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(&count, size);
#endif
    return count;
}