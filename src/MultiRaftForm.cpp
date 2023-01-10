#include <sharpen/MultiRaftForm.hpp>

#include <cstring>

sharpen::MultiRaftForm::MultiRaftForm() noexcept
    :Self{sharpen::RaftMailType::Unknown}
{}

sharpen::MultiRaftForm::MultiRaftForm(sharpen::RaftMailType type) noexcept
    :Self{type,0}
{}

sharpen::MultiRaftForm::MultiRaftForm(sharpen::RaftMailType type,std::uint32_t number) noexcept
    :magic_()
    ,type_(0)
    ,chksum_(0)
    ,number_(number)
{
    assert(sizeof(this->magic_) == multiRaftMagic.GetSize());
    std::memcpy(this->magic_,multiRaftMagic.Data(),sizeof(this->magic_));
    if(type == sharpen::RaftMailType::MaxValue)
    {
        type = sharpen::RaftMailType::Unknown;
    }
    this->type_ = static_cast<std::uint32_t>(type);
}

sharpen::MultiRaftForm::MultiRaftForm(const Self &other) noexcept
    :magic_()
    ,type_(other.type_)
    ,chksum_(other.chksum_)
    ,number_(other.number_)
{
    std::memcpy(this->magic_,other.magic_,sizeof(this->magic_));
}

sharpen::MultiRaftForm::MultiRaftForm(Self &&other) noexcept
    :magic_()
    ,type_(other.type_)
    ,chksum_(other.chksum_)
    ,number_(other.number_)
{
    std::memcpy(this->magic_,other.magic_,sizeof(this->magic_));
    std::memset(other.magic_,0,sizeof(other.magic_));
    other.type_ = 0;
    other.chksum_ = 0;
    other.number_ = 0;
}

sharpen::MultiRaftForm &sharpen::MultiRaftForm::operator=(const Self &other) noexcept
{
    if(this != std::addressof(other))
    {
        std::memcpy(this->magic_,other.magic_,sizeof(this->magic_));
        this->type_ = other.type_;
        this->chksum_ = other.chksum_;
        this->number_ = other.number_;
    }
    return *this;
}

sharpen::MultiRaftForm &sharpen::MultiRaftForm::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        std::memcpy(this->magic_,other.magic_,sizeof(this->magic_));
        this->type_ = other.type_;
        this->chksum_ = other.chksum_;
        this->number_ = other.number_;
        std::memset(other.magic_,0,sizeof(other.magic_));
        other.type_ = 0;
        other.chksum_ = 0;
        other.number_ = 0;
    }
    return *this;
}

void sharpen::MultiRaftForm::SetChecksum(sharpen::ByteSlice slice) noexcept
{
    this->SetChecksum(slice.Crc16());
}

void sharpen::MultiRaftForm::SetChecksum(const char *data,std::size_t size) noexcept
{
    this->SetChecksum(sharpen::Crc16(data,size));
}

bool sharpen::MultiRaftForm::CheckContent(sharpen::ByteSlice content) const noexcept
{
    return content.Crc16() == this->chksum_;
}