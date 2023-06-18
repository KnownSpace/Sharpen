#include <common/RaftTool.hpp>

#include <sharpen/FileOps.hpp>
#include <sharpen/GenericMailParserFactory.hpp>
#include <sharpen/IpTcpActorBuilder.hpp>
#include <sharpen/MultiRaftMailBuilder.hpp>
#include <sharpen/MultiRaftMailExtractor.hpp>
#include <sharpen/Quorum.hpp>
#include <sharpen/RaftConsensus.hpp>
#include <sharpen/RaftLogAccesser.hpp>
#include <sharpen/RaftMailBuilder.hpp>
#include <sharpen/RaftMailExtractor.hpp>
#include <sharpen/WalLogStorage.hpp>
#include <sharpen/WalStatusMap.hpp>
#include <sstream>


std::vector<sharpen::IpEndPoint> GetPeers(std::uint16_t begin, std::uint16_t end) {
    std::vector<sharpen::IpEndPoint> peers;
    peers.reserve(end - begin + 1);
    for (std::size_t i = begin; i != end + 1; ++i) {
        sharpen::IpEndPoint endPoint;
        endPoint.SetAddrByString("127.0.0.1");
        endPoint.SetPort(i);
        peers.emplace_back(std::move(endPoint));
    }
    return peers;
}

std::string FormatName(const char *name, const char *extName, std::uint64_t port) {
    std::stringstream builder;
    builder << name << "." << port << "." << extName;
    return builder.str();
}

std::unique_ptr<sharpen::IStatusMap> CreateStatusMap(std::uint16_t port) {
    std::string walName{FormatName("./raftstatus", "wal", port)};
    std::unique_ptr<sharpen::IStatusMap> status{new (std::nothrow)
                                                    sharpen::WalStatusMap{std::move(walName)}};
    if (!status) {
        std::terminate();
    }
    return status;
}

std::unique_ptr<sharpen::ILogStorage> CreateLogStorage(std::uint16_t port) {
    std::string walName{FormatName("./raftlog", "wal", port)};
    std::unique_ptr<sharpen::ILogStorage> logs{new (std::nothrow)
                                                   sharpen::WalLogStorage{std::move(walName)}};
    if (!logs) {
        std::terminate();
    }
    return logs;
}

void RemoveStatusMap(std::uint16_t port) {
    std::string walName{FormatName("./raftstatus", "wal", port)};
    sharpen::RemoveFile(walName.c_str());
}

void RemoveLogStorage(std::uint16_t port) {
    std::string walName{FormatName("./raftlog", "wal", port)};
    sharpen::RemoveFile(walName.c_str());
}

std::unique_ptr<sharpen::IRaftLogAccesser> CreateLogAccesser(std::uint32_t magic) {
    std::unique_ptr<sharpen::IRaftLogAccesser> accesser{new (std::nothrow)
                                                            sharpen::RaftLogAccesser{magic}};
    if (!accesser) {
        std::terminate();
    }
    return accesser;
}

std::unique_ptr<sharpen::IQuorum> ConfigPeers(sharpen::IQuorum *quorum,
                                              std::uint16_t port,
                                              std::uint16_t begin,
                                              std::uint16_t end,
                                              sharpen::IMailReceiver *receiver,
                                              std::uint32_t magic) {
    assert(receiver != nullptr);
    std::unique_ptr<sharpen::IQuorum> peers{quorum};
    peers.reset(new (std::nothrow) sharpen::Quorum{});
    if (!peers) {
        std::terminate();
    }
    auto ports{GetPeers(begin, end)};
    std::shared_ptr<sharpen::IMailParserFactory> factory{
        std::make_shared<sharpen::GenericMailParserFactory>(magic)};
    for (auto begin = ports.begin(), end = ports.end(); begin != end; ++begin) {
        if (begin->GetPort() != port) {
            std::unique_ptr<sharpen::IpTcpActorBuilder> builder{new (std::nothrow)
                                                                    sharpen::IpTcpActorBuilder{}};
            if (!builder) {
                std::terminate();
            }
            builder->PrepareRemote(*begin);
            builder->PrepareReceiver(*receiver);
            builder->PrepareParserFactory(factory);
            peers->Register(begin->GetActorId(), std::move(builder));
        }
    }
    return peers;
}

std::shared_ptr<sharpen::IConsensus> CreateRaft(
    std::uint16_t port,
    std::uint32_t magic,
    std::unique_ptr<sharpen::IRaftSnapshotController> snapshotCtrl,
    sharpen::RaftOption option,
    bool pipeline) {
    sharpen::IpEndPoint endPoint;
    endPoint.SetAddrByString("127.0.0.1");
    endPoint.SetPort(port);
    std::shared_ptr<sharpen::RaftConsensus> raft{
        std::make_shared<sharpen::RaftConsensus>(endPoint.GetActorId(),
                                                 CreateStatusMap(port),
                                                 CreateLogStorage(port),
                                                 CreateLogAccesser(magic),
                                                 std::move(snapshotCtrl),
                                                 option)};
    std::unique_ptr<sharpen::IRaftMailBuilder> builder{new (std::nothrow)
                                                           sharpen::RaftMailBuilder{magic}};
    if (!builder) {
        std::terminate();
    }
    std::unique_ptr<sharpen::IRaftMailExtractor> extractor{new (std::nothrow)
                                                               sharpen::RaftMailExtractor{magic}};
    if (!extractor) {
        std::terminate();
    }
    raft->PrepareMailBuilder(std::move(builder));
    raft->PrepareMailExtractor(std::move(extractor));
    return raft;
}