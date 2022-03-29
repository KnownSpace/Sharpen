#pragma once
#ifndef _SHARPEN_RAFTID_HPP
#define _SHARPEN_RAFTID_HPP

#include <utility>
#include <functional>

namespace sharpen
{
    template<typename _Endpoint>
    class RaftId
    {
    private:
        using Self = sharpen::RaftId<_Endpoint>;

        _Endpoint endpoint_;
    public:
        RaftId() = default;

        explicit RaftId(const _Endpoint &endpoint)
            :endpoint_(endpoint)
        {}

        RaftId(const Self &other) = default;

        RaftId(Self &&other) noexcept = default;

        ~RaftId() noexcept = default;

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(other,*this);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->endpoint_ = std::move(other.endpoint_);
            }
            return *this;
        }

        _Endpoint &GetEndPoint() noexcept
        {
            return this->endpoint_;
        }

        const _Endpoint &GetEndPoint() const noexcept
        {
            return this->endpoint_;
        }
    };
}

namespace std
{
    template<typename _Endpoint>
    struct hash<sharpen::RaftId<_Endpoint>>
    {
        std::size_t operator()(const sharpen::RaftId<_Endpoint> &id) const noexcept
        {
            return std::hash<_Endpoint>{}(id.GetEndPoint());
        }
    };
}

#endif