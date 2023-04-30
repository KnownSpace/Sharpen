#pragma once
#ifndef _SHARPEN_IRAFTSNAPSHOTCONTROLLER_HPP
#define _SHARPEN_IRAFTSNAPSHOTCONTROLLER_HPP

#include "IRaftSnapshotInstaller.hpp"
#include "IRaftSnapshotProvider.hpp"

namespace sharpen
{
    class IRaftSnapshotController
        : public sharpen::IRaftSnapshotProvider
        , public sharpen::IRaftSnapshotInstaller
    {
    private:
        using Self = sharpen::IRaftSnapshotController;

    protected:
    public:
        IRaftSnapshotController() noexcept = default;

        IRaftSnapshotController(const Self &other) noexcept = default;

        IRaftSnapshotController(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IRaftSnapshotController() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::IRaftSnapshotProvider &Provider() noexcept
        {
            return *this;
        }

        inline const sharpen::IRaftSnapshotProvider &Provider() const noexcept
        {
            return *this;
        }

        inline sharpen::IRaftSnapshotInstaller &Installer() noexcept
        {
            return *this;
        }

        inline const sharpen::IRaftSnapshotInstaller &Installer() const noexcept
        {
            return *this;
        }
    };
}   // namespace sharpen

#endif