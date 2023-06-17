#include <sharpen/RaftLogAccesser.hpp>

#include <sharpen/BinarySerializator.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/ByteOrder.hpp>


sharpen::RaftLogAccesser::RaftLogAccesser(std::uint32_t magic) noexcept
    : magic_(magic) {
}

sharpen::RaftLogAccesser::RaftLogAccesser(Self &&other) noexcept
    : magic_(other.magic_) {
    other.magic_ = 0;
}

sharpen::RaftLogAccesser &sharpen::RaftLogAccesser::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->magic_ = other.magic_;
        other.magic_ = 0;
    }
    return *this;
}

std::uint64_t sharpen::RaftLogAccesser::NviGetTerm(sharpen::ByteSlice logEntry) const noexcept {
    const sharpen::RaftLogHeader *header{
        reinterpret_cast<const sharpen::RaftLogHeader *>(logEntry.Data())};
    return header->GetMagic();
}


bool sharpen::RaftLogAccesser::NviIsRaftEntry(sharpen::ByteSlice logEntry) const noexcept {
    if (logEntry.GetSize() >= sizeof(sharpen::RaftLogHeader)) {
        const sharpen::RaftLogHeader *header{
            reinterpret_cast<const sharpen::RaftLogHeader *>(logEntry.Data())};
        if (header->GetMagic() == this->magic_) {
            if (logEntry.GetSize() != sizeof(*header)) {
                std::uint32_t checksum{logEntry.Sub(sizeof(sharpen::RaftLogHeader)).Crc32()};
                return checksum == header->GetChecksum();
            } else {
                return true;
            }
        }
    }
    return false;
}


sharpen::ByteBuffer sharpen::RaftLogAccesser::NviCreateEntry(sharpen::ByteSlice bytes,
                                                             std::uint64_t term) const {
    sharpen::RaftLogHeader header{this->magic_, bytes.Crc32(), term};
    sharpen::ByteBuffer buf{sizeof(header) + bytes.GetSize()};
    sharpen::BufferWriter writer{buf};
    writer.Write(header);
    if (!bytes.Empty()) {
        writer.Write(bytes.Data(), bytes.GetSize());
    }
    return buf;
}