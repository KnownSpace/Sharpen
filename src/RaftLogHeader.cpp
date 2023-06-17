#include <sharpen/RaftLogHeader.hpp>

#include <sharpen/ByteOrder.hpp>

sharpen::RaftLogHeader::RaftLogHeader() noexcept
    : Self{0, 0, 0} {
}

sharpen::RaftLogHeader::RaftLogHeader(std::uint32_t magic,
                                      std::uint32_t checksum,
                                      std::uint64_t term) noexcept
    : magic_(0)
    , checksum_(0)
    , term_(0) {
    this->SetMagic(magic);
    this->SetChecksum(checksum);
    this->SetTerm(term);
}

sharpen::RaftLogHeader::RaftLogHeader(Self &&other) noexcept
    : magic_(other.magic_)
    , checksum_(other.checksum_)
    , term_(other.term_)  {
    other.magic_ = 0;
    other.checksum_ = 0;
    other.term_ = 0;
}

sharpen::RaftLogHeader &sharpen::RaftLogHeader::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->magic_ = other.magic_;
        this->checksum_ = other.checksum_;
        this->term_ = other.term_;
    }
    return *this;
}

std::uint32_t sharpen::RaftLogHeader::GetMagic() const noexcept {
    std::uint32_t magic{this->magic_};
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(magic);
#endif
    return magic;
}

void sharpen::RaftLogHeader::SetMagic(std::uint32_t magic) noexcept {
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(magic);
#endif
    this->magic_ = magic;
}

std::uint32_t sharpen::RaftLogHeader::GetChecksum() const noexcept {
    std::uint32_t checksum{this->checksum_};
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(checksum);
#endif
    return checksum;
}

void sharpen::RaftLogHeader::SetChecksum(std::uint32_t checksum) noexcept {
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(checksum);
#endif
    this->checksum_ = checksum;
}

std::uint64_t sharpen::RaftLogHeader::GetTerm() const noexcept {
    std::uint64_t term{this->term_};
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(term);
#endif
    return term;    
}

void sharpen::RaftLogHeader::SetTerm(std::uint64_t term) noexcept {
#ifdef SHARPEN_IS_BIG_ENDIAN
    sharpen::ConvertEndian(term);
#endif
    this->term_ = term;
}