#pragma once
#ifndef _SHARPEN_RAFTMEMBER_HPP
#define _SHARPEN_RAFTMEMBER_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    struct RaftMemberInfo
    {
        sharpen::Uint64 matchIndex_;
        sharpen::Uint64 nextIndex_;
    };

    template<typename _Id,typename _Data>
    class InternalRaftMember
    {
    private:
        using Self = sharpen::InternalRaftMember<_Id,_Data>;
        
        _Id id_;
        sharpen::RaftMemberInfo info_;
        _Data data_;
    public:
        explicit InternalRaftMember(_Id id)
            :InternalRaftMember(std::move(id),_Data{},0,0)
        {}

        InternalRaftMember(_Id id,_Data data)
            :InternalRaftMember(std::move(id),std::move(data),0,0)
        {}

        InternalRaftMember(_Id id,_Data data,sharpen::Uint64 matchIndex,sharpen::Uint64 nextIndex)
            :id_(std::move(id))
            ,info_()
            ,data_(std::move(data))
        {
            this->info_.matchIndex_ = matchIndex;
            this->info_.nextIndex_ = nextIndex;
        }

        InternalRaftMember(const Self &other) = default;

        InternalRaftMember(Self &&other) noexcept = default;

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->id_ = std::move(other.id_);
                this->info_ = std::move(other.info_);
                this->data_ = std::move(other.data_);
                other.info_.matchIndex_ = 0;
                other.info_.nextIndex_ = 0;
            }
            return *this;
        }

        _Id &Id() noexcept
        {
            return this->id_;
        }

        const _Id &Id() const noexcept
        {
            return this->id_;
        }

        sharpen::Uint64 GetMatchIndex() const noexcept
        {
            return this->info_.matchIndex_;
        }

        sharpen::Uint64 GetNextIndex() const noexcept
        {
            return this->info_.nextIndex_;
        }

        void SetNextIndex(sharpen::Uint64 nextIndex) noexcept
        {
            this->info_.nextIndex_ = nextIndex;
        }

        void SetMatchIndex(sharpen::Uint64 matchIndex) noexcept
        {
            this->info_.matchIndex_ = matchIndex;
        }

        _Data &Data() noexcept
        {
            return this->data_;
        }

        const _Data &Data() const noexcept
        {
            return this->data_;
        }

        ~InternalRaftMember() noexcept = default;
    };

    template<typename _Id>
    class InternalRaftMember<_Id,void>
    {
    private:
        using Self = sharpen::InternalRaftMember<_Id,void>;
        
        _Id id_;
        sharpen::RaftMemberInfo info_;
    public:
        explicit InternalRaftMember(_Id id)
            :InternalRaftMember(std::move(id),0,0)
        {}

        InternalRaftMember(_Id id,sharpen::Uint64 matchIndex,sharpen::Uint64 nextIndex)
            :id_(std::move(id))
            ,info_()
        {
            this->info_.matchIndex_ = matchIndex;
            this->info_.nextIndex_ = nextIndex;
        }

        InternalRaftMember(const Self &other) = default;

        InternalRaftMember(Self &&other) noexcept = default;

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->id_ = std::move(other.id_);
                this->info_ = std::move(other.info_);
                other.info_.matchIndex_ = 0;
                other.info_.nextIndex_ = 0;
            }
            return *this;
        }

        _Id &Id() noexcept
        {
            return this->id_;
        }

        const _Id &Id() const noexcept
        {
            return this->id_;
        }

        sharpen::Uint64 GetMatchIndex() const noexcept
        {
            return this->info_.matchIndex_;
        }

        sharpen::Uint64 GetNextIndex() const noexcept
        {
            return this->info_.nextIndex_;
        }

        void SetNextIndex(sharpen::Uint64 nextIndex) noexcept
        {
            this->info_.nextIndex_ = nextIndex;
        }

        void SetMatchIndex(sharpen::Uint64 matchIndex) noexcept
        {
            this->info_.matchIndex_ = matchIndex;
        }

        ~InternalRaftMember() noexcept = default;
    };

    template<typename _Id,typename _Data = void>
    using RaftMember = sharpen::InternalRaftMember<_Id,_Data>;
}

#endif