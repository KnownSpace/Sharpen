#pragma once
#ifndef _SHARPEN_WORKERGROUP_HPP
#define _SHARPEN_WORKERGROUP_HPP

#include "AsyncBlockingQueue.hpp"
#include "IFiberScheduler.hpp"
#include "IWorkerGroup.hpp"
#include "NoexceptInvoke.hpp" // IWYU pragma: keep
#include <vector>

namespace sharpen
{
    class FixedWorkerGroup
        : public sharpen::IWorkerGroup
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::FixedWorkerGroup;

        void Entry(std::size_t index) noexcept;

        virtual void NviSubmit(std::function<void()> task) override;

        std::atomic_bool token_;
        sharpen::AsyncBlockingQueue<std::function<void()>> queue_;
        std::vector<sharpen::AwaitableFuture<void>> workers_;

    public:
        FixedWorkerGroup();

        explicit FixedWorkerGroup(sharpen::IFiberScheduler &scheduler);

        FixedWorkerGroup(sharpen::IFiberScheduler &scheduler, std::size_t workerCount);

        virtual ~FixedWorkerGroup() noexcept
        {
            this->Stop();
            this->Join();
        }

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Join() noexcept override;

        virtual void Stop() noexcept override;

        inline virtual bool Running() const noexcept override
        {
            return this->token_.load();
        }

        inline virtual std::size_t GetWorkerCount() const noexcept override
        {
            return this->workers_.size();
        }
    };
}   // namespace sharpen

#endif