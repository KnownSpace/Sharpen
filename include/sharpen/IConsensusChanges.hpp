#pragma once
#ifndef _SHARPEN_ICONSENSUSCHANGES_HPP
#define _SHARPEN_ICONSENSUSCHANGES_HPP

#include <cstdint>
#include <cstddef>
#include <stdexcept>

#include "ByteBuffer.hpp"
#include "MachineSet.hpp"

namespace sharpen
{
    class IConsensusChanges
    {
    private:
        using Self = sharpen::IConsensusChanges;
    protected:

        virtual void NviInsertMachine(std::uint64_t actorId,sharpen::ByteBuffer log) = 0;

        virtual void NviRemoveMachine(std::uint64_t actorId,sharpen::ByteBuffer log) = 0;

        virtual void NviMoveToBindedBatch() = 0;

        inline static void CheckData(const sharpen::ByteBuffer &log)
        {
            assert(!log.Empty());
            if(log.Empty())
            {
                throw std::invalid_argument{"log could not be empty"};
            }
        }
    public:
    
        IConsensusChanges() noexcept = default;
    
        IConsensusChanges(const Self &other) noexcept = default;
    
        IConsensusChanges(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IConsensusChanges() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual bool Insertable() const noexcept = 0;

        virtual bool Removeable() const noexcept = 0;

        inline void InsertMachine(std::uint64_t actorId,sharpen::ByteBuffer log)
        {
            this->CheckData(log);
            if(!this->Insertable())
            {
                throw std::logic_error{"cannot insert to machine set"};
            }
            this->NviInsertMachine(actorId,std::move(log));
        }

        inline void RemoveMachine(std::uint64_t actorId,sharpen::ByteBuffer log)
        {
            this->CheckData(log);
            if(!this->Removeable())
            {
                throw std::logic_error{"cannot remove from machine set"};
            }
            this->NviRemoveMachine(actorId,std::move(log));
        }

        virtual const sharpen::MachineSet &GetInsertSet() const noexcept = 0;

        virtual const sharpen::MachineSet &GetRemoveSet() const noexcept = 0;

        inline void MoveToBindedBatch()
        {
            if(this->GetInsertSet().Empty() && this->GetRemoveSet().Empty())
            {
                return;
            }
            this->NviMoveToBindedBatch();
        }
    };
}

#endif