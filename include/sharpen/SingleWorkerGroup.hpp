#pragma once
#ifndef _SHARPEN_SINGLEWORKERGROUP_HPP
#define _SHARPEN_SINGLEWORKERGROUP_HPP

#include "AsyncBlockingQueue.hpp"
#include "IFiberScheduler.hpp"
#include "IWorkerGroup.hpp"
#include "NoexceptInvoke.hpp"   // IWYU pragma: keep
#include <functional>

namespace sharpen {
    class SingleWorkerGroup
        : public sharpen::IWorkerGroup
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::SingleWorkerGroup;

        std::atomic_bool token_;
        sharpen::AsyncBlockingQueue<std::function<void()>> queue_;
        sharpen::AwaitableFuture<void> worker_;

        void Entry() noexcept;

        virtual void NviSubmit(std::function<void()> task) override;

        virtual void NviSubmitUrgent(std::function<void ()> task) override;

    public:
        SingleWorkerGroup();

        explicit SingleWorkerGroup(sharpen::IFiberScheduler &scheduler);

        virtual ~SingleWorkerGroup() noexcept;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void Stop() noexcept override;

        virtual void Join() noexcept override;

        virtual bool Running() const noexcept override;

        inline virtual std::size_t GetWorkerCount() const noexcept override {
            return 1;
        }
    };
}   // namespace sharpen

#endif