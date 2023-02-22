#pragma once
#ifndef _SHARPEN_RAFTCONSENSUSCHANGES_HPP
#define _SHARPEN_RAFTCONSENSUSCHANGES_HPP

#include "IConsensusChanges.hpp"
#include "ILogBatch.hpp"

namespace sharpen
{
    class RaftConsensusChanges:public sharpen::IConsensusChanges
    {
    private:
        using Self = sharpen::RaftConsensusChanges;
    
    public:
        enum class Mode
        {
            Prepare,
            Apply
        };

    private:

        Mode mode_;
        sharpen::MachineSet insertSet_;
        sharpen::MachineSet removeSet_;
        sharpen::ILogBatch *logBatch_;

        virtual void NviInsertMachine(std::uint64_t actorId,sharpen::ByteBuffer log) override;

        virtual void NviRemoveMachine(std::uint64_t actorId,sharpen::ByteBuffer log) override;

        virtual void NviMoveToBindedBatch() override;
    public:
    
        RaftConsensusChanges(Mode mode,sharpen::ILogBatch &batch);
    
        RaftConsensusChanges(const Self &other) = default;
    
        RaftConsensusChanges(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~RaftConsensusChanges() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual bool Insertable() const noexcept override;

        virtual bool Removeable() const noexcept override;

        virtual const sharpen::MachineSet &GetInsertSet() const noexcept override;

        virtual const sharpen::MachineSet &GetRemoveSet() const noexcept override;
    };
}

#endif