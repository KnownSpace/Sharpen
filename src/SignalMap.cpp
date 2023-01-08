#include <sharpen/SignalMap.hpp>

#include <sharpen/SystemMacro.hpp>

#include <cassert>
#include <algorithm>

#include <csignal>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

sharpen::SignalMap::SignalMap()
    :map_()
    ,remap_()
    ,lock_()
{}

void sharpen::SignalMap::Register(sharpen::FileHandle handle,std::int32_t *sigs,std::size_t sigSize)
{
    {
        std::unique_lock<Lock> lock{this->lock_};
        //register to map
        for(std::size_t i = 0;i != sigSize;++i)
        {
            std::int32_t sig{sigs[i]};
            auto ite = this->map_.find(sig);
            if(ite != this->map_.end())
            {
                bool exist{std::find(ite->second.begin(),ite->second.end(),handle) != ite->second.end()};
                if(!exist)
                {
                    ite->second.emplace_back(handle);
                }
            }
            else
            {
                std::vector<sharpen::FileHandle> handles;
                handles.resize(1);
                handles[0] = handle;
                this->map_.emplace(sig,std::move(handles));
            }
        }
        //register to remap
        auto ite = this->remap_.find(handle);
        if(ite != this->remap_.end())
        {
            for(std::size_t i = 0;i != sigSize;++i)
            {
                std::int32_t sig{sigs[i]};
                bool exist{std::find(ite->second.begin(),ite->second.end(),sig) != ite->second.end()};
                if(!exist)
                {
                    ite->second.emplace_back(sig);
                }
            }
        }
        else
        {
            std::vector<std::int32_t> sigVec;
            sigVec.resize(sigSize);
            for(std::size_t i = 0;i != sigSize;++i)
            {
                sigVec[i] = sigs[i];   
            }
            this->remap_.emplace(handle,std::move(sigVec));
        }
    }
}

void sharpen::SignalMap::Raise(std::int32_t sig) const noexcept
{
    bool raiseDefault{true};
    {
        std::unique_lock<Lock> lock{this->lock_};
        auto ite = this->map_.find(sig);
        if(ite != this->map_.end())
        {
            raiseDefault = false;
            for(auto begin = ite->second.begin(),end = ite->second.end(); begin != end; ++begin)
            {
                std::uint8_t sigBit{static_cast<std::uint8_t>(sig)};
    #ifdef SHARPEN_IS_WIN
                BOOL r{::WriteFile(*begin,&sigBit,sizeof(sigBit),nullptr,nullptr)};
                (void)r;
    #else
                ssize_t size{-1};
                do
                {
                    size = ::write(*begin,&sigBit,sizeof(sigBit));
                } while (size == -1 && errno == EINTR);
                (void)size;
    #endif   
            }
        }
    }
    if(raiseDefault)
    {
        SIG_DFL(sig);
    }
}

void sharpen::SignalMap::Unregister(sharpen::FileHandle handle) noexcept
{
    {
        std::unique_lock<Lock> lock{this->lock_};
        auto ite = this->remap_.find(handle);
        if(ite != this->remap_.end())
        {
            for(auto begin = ite->second.begin(),end = ite->second.end(); begin != end; ++begin)
            {
                std::int32_t sig{*begin};
                auto it = this->map_.find(sig);
                assert(it != this->map_.end());
                auto handleIt = std::find(it->second.begin(),it->second.end(),handle);
                if(handleIt != it->second.end())
                {
                    it->second.erase(handleIt);
                }
            }
            this->remap_.erase(ite);
        }
    }
}