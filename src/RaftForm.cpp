#include <sharpen/RaftForm.hpp>

#include <cstring>

sharpen::RaftForm::RaftForm() noexcept
    :Self{sharpen::RaftMailType::Unknown}
{}

sharpen::RaftForm::RaftForm(sharpen::RaftMailType type) noexcept
    :magic_(raftMagic)
    ,type_(0)
    ,chksum_(0)
    ,padding_()
{
    if(type == sharpen::RaftMailType::MaxValue)
    {
        type = sharpen::RaftMailType::Unknown;
    }
    this->type_ = static_cast<std::uint32_t>(type);
    std::memset(this->padding_,0,sizeof(this->padding_));
}

sharpen::RaftForm::RaftForm(const Self &other) noexcept
    :magic_(other.magic_)
    ,type_(other.type_)
    ,chksum_(other.chksum_)
    ,padding_()
{
    std::memcpy(this->padding_,other.padding_,sizeof(this->padding_));
}

sharpen::RaftForm::RaftForm(Self &&other) noexcept
    :magic_(other.magic_)
    ,type_(other.type_)
    ,chksum_(other.chksum_)
    ,padding_()
{
    other.magic_ = 0;
    other.type_ = 0;
    other.chksum_ = 0;
    std::memcpy(this->padding_,other.padding_,sizeof(this->padding_));
}

sharpen::RaftForm &sharpen::RaftForm::operator=(const Self &other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        this->type_ = other.type_;
        this->chksum_ = other.chksum_;
        std::memcpy(this->padding_,other.padding_,sizeof(this->padding_));
    }
    return *this;
}

sharpen::RaftForm &sharpen::RaftForm::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->magic_ = other.magic_;
        this->type_ = other.type_;
        this->chksum_ = other.chksum_;
        std::memcpy(this->padding_,other.padding_,sizeof(this->padding_));
        other.magic_ = 0;
        other.type_ = 0;
        other.chksum_ = 0;
    }
    return *this;
}

void sharpen::RaftForm::SetChecksum(sharpen::ByteSlice slice) noexcept
{
    this->SetChecksum(slice.Crc16());
}

void sharpen::RaftForm::SetChecksum(const char *data,std::size_t size) noexcept
{
    this->SetChecksum(sharpen::Crc16(data,size));
}

bool sharpen::RaftForm::CheckContent(sharpen::ByteSlice content) const noexcept
{
    return content.Crc16() == this->chksum_;
}