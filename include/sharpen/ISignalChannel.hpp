#pragma once
#ifndef _SHARPEN_ISIGNALCHANNEL_HPP
#define _SHARPEN_ISIGNALCHANNEL_HPP

#include "Future.hpp"
#include "IChannel.hpp"
#include "SignalBuffer.hpp"
#include "SignalFd.hpp"
#include "SignalMap.hpp"
#include <cstddef>
#include <cstdint>

// define SHARPEN_USE_PIPESIGNAL to force sharpen use pipe(on linux).
#if (defined(SHARPEN_HAS_SIGNALFD)) && (!defined(SHARPEN_USE_PIPESIGNAL))
#define SHARPEN_USE_SIGNALFD
#elif (!defined(SHARPEN_USE_PIPESIGNAL))
#define SHARPEN_USE_PIPESIGNAL
#endif

#ifdef SHARPEN_USE_PIPESIGNAL
#ifdef __cplusplus
extern "C"
{
#endif

    // Until C++17:
    // Signal handlers are expected to have C linkage and, in general, only use the features from
    // the common subset of C and C++. It is implementation-defined if a function with C++ linkage
    // can be used as a signal handler.
    extern void SHARPEN_SignalHandler(int sig);

#ifdef __cplusplus
}
#endif
#endif

namespace sharpen
{

#ifdef SHARPEN_USE_PIPESIGNAL

    class SignalStorage
    {
    private:
        using Self = sharpen::SignalStorage;

        static void DoInstallHandler();

    protected:
    public:
        SignalStorage() noexcept = default;

        SignalStorage(const Self &other) noexcept = default;

        SignalStorage(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        ~SignalStorage() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        static std::unique_ptr<sharpen::SignalMap> sigmap;

        static std::once_flag sigmapFlag;

        static std::atomic_uint64_t sigBitset;

        static void InstallHandler(std::int32_t sig);
    };

#endif

    class ISignalChannel : public sharpen::IChannel
    {
    private:
        using Self = sharpen::ISignalChannel;

    protected:
    public:
        ISignalChannel() noexcept = default;

        virtual ~ISignalChannel() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void ReadAsync(sharpen::SignalBuffer &signals,
                               sharpen::Future<std::size_t> &future) = 0;

        std::size_t ReadAsync(sharpen::SignalBuffer &signals);
    };

    using SignalChannelPtr = std::shared_ptr<sharpen::ISignalChannel>;

    extern SignalChannelPtr OpenSignalChannel(std::int32_t *sig, std::size_t sigCount);
}   // namespace sharpen

#endif