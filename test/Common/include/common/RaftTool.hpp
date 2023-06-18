#pragma once
#ifndef _RAFTTOOL_HPP
#define _RAFTTOOL_HPP

#include <sharpen/IConsensus.hpp>
#include <sharpen/ILogStorage.hpp>
#include <sharpen/IMailReceiver.hpp>
#include <sharpen/IRaftLogAccesser.hpp>
#include <sharpen/IRaftSnapshotController.hpp>
#include <sharpen/IStatusMap.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/Quorum.hpp>
#include <sharpen/RaftOption.hpp>
#include <memory>
#include <string>
#include <vector>




extern std::vector<sharpen::IpEndPoint> GetPeers(std::uint16_t begin, std::uint16_t end);

extern std::string FormatName(const char *name, const char *extName, std::uint64_t port);

extern std::unique_ptr<sharpen::IStatusMap> CreateStatusMap(std::uint16_t port);

extern std::unique_ptr<sharpen::ILogStorage> CreateLogStorage(std::uint16_t port);

extern void RemoveStatusMap(std::uint16_t port);

extern void RemoveLogStorage(std::uint16_t port);

extern std::unique_ptr<sharpen::IRaftLogAccesser> CreateLogAccesser(std::uint32_t magic);

extern std::unique_ptr<sharpen::IQuorum> ConfigPeers(sharpen::IQuorum *quorum,
                                                     std::uint16_t port,
                                                     std::uint16_t begin,
                                                     std::uint16_t end,
                                                     sharpen::IMailReceiver *receiver,
                                                     std::uint32_t magic);

extern std::shared_ptr<sharpen::IConsensus> CreateRaft(
    std::uint16_t port,
    std::uint32_t magic,
    std::unique_ptr<sharpen::IRaftSnapshotController> snapshotCtrl,
    sharpen::RaftOption option);

#endif