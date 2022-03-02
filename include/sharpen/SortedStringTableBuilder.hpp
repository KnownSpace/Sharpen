#pragma once
#ifndef _SHARPEN_SORTEDSTRINGTABLEBUILDER_HPP
#define _SHARPEN_SORTEDSTRINGTABLEBUILDER_HPP

#include "SstDataBlock.hpp"
#include "BloomFilter.hpp"
#include "SstRoot.hpp"
#include "SortedStringTableBuilderConcept.hpp"
#include "SstVector.hpp"

namespace sharpen
{
    struct SortedStringTableBuilder
    {
    private:
        template<typename _Iterator>
        struct Helper
        {
            template<typename _Block>
            static void PutWalKv(_Block &block,_Iterator ite)
            {
                if(!ite->second.IsDeleted())
                {
                    block.Put(ite->first,ite->second.Value());
                }
                else
                {
                    block.Delete(ite->first);
                }
            }

            static void AssignKv(_Iterator ite,sharpen::ByteBuffer &key,sharpen::ByteBuffer &value)
            {
                key = ite->first;
                value = ite->second.Value();
            }
        };

        template<typename _Iterator>
        struct Helper<std::move_iterator<_Iterator>>
        {
            template<typename _Block>
            static void PutWalKv(_Block &block,std::move_iterator<_Iterator> ite)
            {
                if(!ite->second.IsDeleted())
                {
                    block.Put(std::move(ite->first),std::move(ite->second.Value()));
                }
                else
                {
                    block.Delete(std::move(ite->first));
                }
            }

            static void AssignKv(std::move_iterator<_Iterator> ite,sharpen::ByteBuffer &key,sharpen::ByteBuffer &value)
            {
                key = std::move(ite->first);
                value = std::move(ite->second.Value());
            }
        };

        template<typename _Block,typename _Check = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>>
        static bool GetKv(_Block &block,sharpen::Size &groupIndex,sharpen::Size &keyIndex,sharpen::ByteBuffer &key,sharpen::ByteBuffer &value)
        {
            while (groupIndex != block.GetSize())
            {
                auto &group = *sharpen::IteratorForward(block.Begin(),groupIndex);
                while (keyIndex != group.GetSize())
                {
                    auto kv = sharpen::IteratorForward(group.Begin(),keyIndex);
                    key = std::move(*kv).MoveKey();
                    value = std::move(kv->Value());
                    ++keyIndex;
                    return true;
                }
                keyIndex = 0;
                ++groupIndex;
            }
            return false;
        }

        template<typename _Block>
        static void WriteBlock(const _Block &block,sharpen::FileChannelPtr channel,sharpen::SstRoot &root,sharpen::ByteBuffer &buf,sharpen::Size filterBits,std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> &filters,sharpen::Uint64 &offset)
        {
            if(filterBits)
            {
                filters.emplace_back(Self::BuildFilter(block,filterBits));
                root.MetaIndexBlock().Put(block.LastKey(),sharpen::FilePointer{});
            }
            sharpen::ByteBuffer lastKey{block.LastKey()};
            try
            {
                sharpen::Size size{block.StoreTo(buf)};
                channel->WriteAsync(buf.Data(),size,offset);
                sharpen::FilePointer pointer;
                pointer.offset_ = offset;
                pointer.size_ = size;
                root.IndexBlock().Put(std::move(lastKey),pointer);
                offset += size;
            }
            catch(const std::exception&)
            {
                channel->Truncate();
                throw;
            }
        }

        static void WriteFilters(sharpen::FileChannelPtr channel,const std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> &filters,sharpen::SstRoot &root,sharpen::Uint64 &offset,sharpen::ByteBuffer &buf);

        using Self = sharpen::SortedStringTableBuilder;

        struct TableState
        {
            sharpen::Size blockIndex_;
            sharpen::Size groupIndex_;
            sharpen::Size keyIndex_;  
        };

        inline static sharpen::FileChannelPtr GetChannel(sharpen::FileChannelPtr channel)
        {
            return channel;
        }
    public:
        template<typename _Block,typename _Check = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>>
        static sharpen::BloomFilter<sharpen::ByteBuffer> BuildFilter(const _Block &block,sharpen::Size bits)
        {
            sharpen::BloomFilter<sharpen::ByteBuffer> filter{block.ComputeSize()*bits,bits};
            for (auto groupBegin = block.Begin(),groupEnd = block.End(); groupBegin != groupEnd; ++groupBegin)
            {
                for (auto keyBegin = groupBegin->Begin(),keyEnd = groupBegin->End(); keyBegin != keyEnd; ++keyBegin)
                {
                    filter.Add(keyBegin->GetKey());   
                }
            }
            return filter;
        }

        template<typename _Block,typename _Check = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>>
        static _Block LoadDataBlock(sharpen::FileChannelPtr channel,sharpen::Uint64 offset,sharpen::Uint64 size)
        {
            _Block block;
            sharpen::ByteBuffer buf{sharpen::IntCast<sharpen::Size>(size)};
            channel->ReadAsync(buf,offset);
            block.LoadFrom(buf);
            return block;
        }

        static sharpen::BloomFilter<sharpen::ByteBuffer> LoadFilter(sharpen::FileChannelPtr channel,sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::Size bitsOfElements);

        //dump wal table to sst
        template<typename _Block,typename _Iterator,typename _Check = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value && sharpen::IsSstDataBlock<_Block>::Value>>
        static sharpen::SstRoot DumpWalToTable(sharpen::FileChannelPtr channel,sharpen::Size blockBytes,_Iterator begin,_Iterator end,sharpen::Size filterBits,bool eraseDeleted)
        {
            assert(blockBytes);
            //build data block
            sharpen::SstRoot root;
            std::vector<_Block> blocks;
            std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> filters;
            blocks.reserve(8);
            sharpen::Size blockSize{0};
            while (begin != end)
            {
                if(blocks.empty() || blockSize > blockBytes)
                {
                    blockSize = 0;
                    blocks.emplace_back();
                }
                blockSize += begin->first.GetSize();
                blockSize += begin->second.Value().GetSize();
                Self::Helper<_Iterator>::PutWalKv(blocks.back(),begin);
                ++begin;
            }
            if(filterBits)
            {
                filters.reserve(blocks.size());
                root.MetaIndexBlock().Reserve(blocks.size());
            }
            root.IndexBlock().Reserve(blocks.size());
            sharpen::ByteBuffer buf;
            sharpen::Uint64 offset{0};
            //foreach blocks
            for (auto blockBegin = blocks.begin(),blockEnd = blocks.end(); blockBegin != blockEnd; ++blockBegin)
            {
                if(eraseDeleted)
                {
                    blockBegin->EraseDeleted();
                }
                Self::WriteBlock(*blockBegin,channel,root,buf,filterBits,filters,offset);
            }
            sharpen::Size index{0};
            if(filterBits)
            {
                Self::WriteFilters(channel,filters,root,offset,buf);
                filters.clear();
                filters.shrink_to_fit();
            }
            blocks.clear();
            blocks.shrink_to_fit();
            //write footer
            root.StoreTo(channel,offset);
            return root;
        }

        //Combine ordered tables
        template<typename _Block,typename _Iterator,typename _Check = decltype(std::declval<sharpen::SstVector&>() = *std::declval<_Iterator>()),typename _CheckBlock = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>>
        static sharpen::SstRoot CombineTables(sharpen::FileChannelPtr channel,_Iterator begin,_Iterator end,sharpen::Size filterBits,bool eraseDeleted)
        {
            sharpen::SstRoot root;
            std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> filters;
            sharpen::Size blockNum{0};
            for (auto ite = begin; ite != end; ++ite)
            {
                blockNum += begin->Root().IndexBlock().GetSize();   
            }
            root.IndexBlock().Reserve(blockNum);
            if(filterBits)
            {
                filters.reserve(blockNum);
                root.MetaIndexBlock().Reserve(blockNum);
            }
            sharpen::ByteBuffer buf;
            sharpen::Uint64 offset{0};
            while (begin != end)
            {
                auto &table = begin->Root();
                for (auto blockBegin = table.IndexBlock().Begin(),blockEnd = table.IndexBlock().End(); blockBegin != blockEnd; ++blockBegin)
                {
                    _Block block{Self::LoadDataBlock<_Block>(begin->Channel(),blockBegin->Block().offset_,blockBegin->Block().size_)};
                    if(eraseDeleted)
                    {
                        block.EraseDeleted();
                    }
                    Self::WriteBlock(block,channel,root,buf,filterBits,filters,offset);
                }
            }
            //write filter blocks
            if(filterBits)
            {
                Self::WriteFilters(channel,filters,root,offset,buf);
            }
            root.StoreTo(channel,offset);
            return root;
        }

        //2PMMS
        template<typename _Block,typename _Iterator,typename _WalIterator,typename _Check = decltype(std::declval<sharpen::SstVector&>() = *std::declval<_Iterator>()),typename _CheckBlock = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>,typename _CheckWalIterator = sharpen::EnableIf<sharpen::IsWalKeyValuePairIterator<_Iterator>::Value && sharpen::IsSstDataBlock<_Block>::Value>>
        static sharpen::SstRoot MergeTables(sharpen::FileChannelPtr channel,sharpen::Size blockBytes,_Iterator begin,_Iterator end,_WalIterator walBegin,_WalIterator walEnd,sharpen::Size filterBits,bool eraseDeleted)
        {
            assert(blockBytes != 0);
            sharpen::SstRoot root;
            sharpen::Size len{sharpen::GetRangeSize(begin,end)};
            if(len)
            {
                //init table blocks
                sharpen::Size reserveLen{len};
                if(walBegin != walEnd)
                {
                    reserveLen += 1;
                }
                std::vector<_Block> blocks{reserveLen};
                std::vector<sharpen::ByteBuffer> keys{reserveLen};
                std::vector<sharpen::ByteBuffer> values{reserveLen};
                sharpen::ByteBuffer *selectedKey{nullptr};
                sharpen::ByteBuffer *selectedValue{nullptr};
                _Block block;
                sharpen::ByteBuffer buf;
                std::vector<Self::TableState> states{len};
                //init block states
                for (auto stateBegin = states.begin(),stateEnd = states.end(); stateBegin != stateEnd; ++stateBegin)
                {
                    std::memset(&(*stateBegin),0,sizeof(*stateBegin));
                }
                //compute blocks count
                sharpen::Size blockNum{0};
                for (auto ite = begin; ite != end; ++ite)
                {
                    sharpen::SstVector &vec = *ite;
                    blockNum += vec.Root().IndexBlock().GetSize();
                }
                if(walBegin != walEnd)
                {
                    blockNum += 1;    
                }
                root.IndexBlock().Reserve(blockNum);
                std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> filters;
                if(filterBits)
                {
                    root.MetaIndexBlock().Reserve(blockNum);
                    filters.reserve(blockNum);
                }
                sharpen::Size blockSize{0};
                sharpen::Uint64 offset{0};
                bool flag{true};
                while (flag)
                {
                    //load key and value from tables
                    for (sharpen::Size i = 0; i != len;)
                    {
                        if(!keys[i].Empty())
                        {
                            ++i;
                            continue;
                        }
                        sharpen::SstVector &vec = *sharpen::IteratorForward(begin,i);
                        if (blocks[i].Empty())
                        {
                            sharpen::Size index{states[i].blockIndex_};
                            if(index != vec.Root().IndexBlock().GetSize())
                            {
                                auto pointer = vec.Root().IndexBlock()[index].Block();
                                blocks[i] = Self::LoadDataBlock<_Block>(vec.Channel(),pointer.offset_,pointer.size_);
                                states[i].groupIndex_ = 0;
                                states[i].keyIndex_ = 0;
                                states[i].blockIndex_ += 1;
                            }  
                        }
                        if(!blocks[i].Empty())
                        {
                            if(!Self::GetKv(blocks[i],states[i].groupIndex_,states[i].keyIndex_,keys[i],values[i]))
                            {
                                blocks[i].Clear();
                                continue;
                            }
                        }
                        ++i;
                    }
                    //load key and value from wal
                    if(walBegin != walEnd && keys.back().Empty())
                    {
                        Self::Helper<_WalIterator>::AssignKv(walBegin,keys.back(),values.back());
                        ++walBegin;
                    }
                    //select key and value
                    for (sharpen::Size i = 0,count = keys.size(); i != count; ++i)
                    {
                        if(!keys[i].Empty() && (!selectedKey || keys[i] <= *selectedKey))
                        {
                            assert(!keys[i].Empty());
                            if(keys[i] == *selectedKey)
                            {
                                selectedKey->Clear();
                            }
                            selectedKey = &keys[i];
                            selectedValue = &values[i];
                        }
                    }
                    //move key and value to block
                    if(selectedKey)
                    {
                        if(block.Contain(*selectedKey))
                        {
                            selectedKey->Clear();
                            selectedKey = nullptr;
                            continue;
                        }
                        blockSize += selectedKey->GetSize();
                        blockSize += selectedValue->GetSize();
                        assert(!selectedKey->Empty());
                        block.Put(std::move(*selectedKey),std::move(*selectedValue));
                        selectedKey = nullptr;
                    }
                    else
                    {
                        flag = false;
                    }
                    if(blockSize >= blockBytes || !flag)
                    {
                        if(eraseDeleted)
                        {
                            block.EraseDeleted();
                        }
                        Self::WriteBlock(block,channel,root,buf,filterBits,filters,offset);
                        blockSize = 0;
                        block.Clear();
                    }
                }
                if(filterBits)
                {
                    Self::WriteFilters(channel,filters,root,offset,buf);
                }
                root.StoreTo(channel,offset);
            }
            else if(walBegin != walEnd)
            {
                return Self::DumpWalToTable<_Block>(std::move(channel),blockBytes,walBegin,walEnd,filterBits,eraseDeleted);
            }
            return root;
        }

        //2PMMS
        template<typename _Block,typename _Iterator,typename _InsertIterator,typename _Check = decltype(std::declval<sharpen::SstVector&>() = *std::declval<_Iterator>()),typename _CheckBlock = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>,typename _CheckInsertor = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::SstRoot&>())>
        static void MergeTables(std::function<sharpen::FileChannelPtr()> channelMaker,sharpen::Size blockBytes,sharpen::Size tableBlocks,_Iterator begin,_Iterator end,sharpen::Size filterBits,bool eraseDeleted,_InsertIterator insertor)
        {
            assert(blockBytes != 0);
            sharpen::Size len{sharpen::GetRangeSize(begin,end)};
            if(len)
            {
                sharpen::SstRoot root;
                std::vector<_Block> blocks{len};
                std::vector<sharpen::ByteBuffer> keys{len};
                std::vector<sharpen::ByteBuffer> values{len};
                sharpen::ByteBuffer *selectedKey{nullptr};
                sharpen::ByteBuffer *selectedValue{nullptr};
                _Block block;
                sharpen::ByteBuffer buf;
                std::vector<Self::TableState> states{len};
                for (auto stateBegin = states.begin(),stateEnd = states.end(); stateBegin != stateEnd; ++stateBegin)
                {
                    std::memset(&(*stateBegin),0,sizeof(*stateBegin));
                }
                root.IndexBlock().Reserve(tableBlocks);
                std::vector<sharpen::BloomFilter<sharpen::ByteBuffer>> filters;
                if(filterBits)
                {
                    root.MetaIndexBlock().Reserve(tableBlocks);
                    filters.reserve(tableBlocks);
                }
                sharpen::Size blockSize{0};
                sharpen::Uint64 offset{0};
                sharpen::Size blockNum{0};
                bool flag{true};
                sharpen::FileChannelPtr channel = channelMaker();
                while (flag)
                {
                    for (sharpen::Size i = 0; i != len;)
                    {
                        if(!keys[i].Empty())
                        {
                            ++i;
                            continue;
                        }
                        sharpen::SstVector &vec = *sharpen::IteratorForward(begin,i);
                        if (blocks[i].Empty())
                        {
                            sharpen::Size index{states[i].blockIndex_};
                            if(index != vec.Root().IndexBlock().GetSize())
                            {
                                auto pointer = vec.Root().IndexBlock()[index].Block();
                                blocks[i] = Self::LoadDataBlock<_Block>(vec.Channel(),pointer.offset_,pointer.size_);
                                states[i].groupIndex_ = 0;
                                states[i].keyIndex_ = 0;
                                states[i].blockIndex_ += 1;
                            }  
                        }
                        if(!blocks[i].Empty())
                        {
                            if(!Self::GetKv(blocks[i],states[i].groupIndex_,states[i].keyIndex_,keys[i],values[i]))
                            {
                                blocks[i].Clear();
                                continue;
                            }
                        }
                        ++i;
                    }
                    //select key and value
                    for (sharpen::Size i = 0; i != len; ++i)
                    {
                        if(!keys[i].Empty() && (!selectedKey || keys[i] <= *selectedKey))
                        {
                            assert(!keys[i].Empty());
                            if(selectedKey && keys[i] == *selectedKey)
                            {
                                selectedKey->Clear();
                            }
                            selectedKey = &keys[i];
                            selectedValue = &values[i];
                        }
                    }
                    //move k,v to block
                    if(selectedKey)
                    {
                        if(block.Contain(*selectedKey))
                        {
                            selectedKey->Clear();
                            selectedKey = nullptr;
                            continue;
                        }
                        blockSize += selectedKey->GetSize();
                        blockSize += selectedValue->GetSize();
                        assert(!selectedKey->Empty());
                        block.Put(std::move(*selectedKey),std::move(*selectedValue));
                        selectedKey = nullptr;
                    }
                    else
                    {
                        flag = false;
                    }
                    if(blockSize >= blockBytes || !flag)
                    {
                        if(eraseDeleted)
                        {
                            block.EraseDeleted();
                        }
                        Self::WriteBlock(block,channel,root,buf,filterBits,filters,offset);
                        blockSize = 0;
                        block.Clear();
                        blockNum += 1;
                    }
                    if(tableBlocks && (blockNum == tableBlocks || !flag))
                    {
                        if(filterBits)
                        {
                            Self::WriteFilters(channel,filters,root,offset,buf);
                        }
                        root.StoreTo(channel,offset);
                        *insertor++ = root;
                        channel = channelMaker();
                        root.IndexBlock().Clear();
                        root.MetaIndexBlock().Clear();
                        offset = 0;
                        blockNum = 0;
                    }
                }
                if (!tableBlocks)
                {
                    if(filterBits)
                    {
                        Self::WriteFilters(channel,filters,root,offset,buf);
                    }
                    root.StoreTo(channel,offset);
                    *insertor++ = root;
                }
            }
        }

        //2PMMS
        template<typename _Block,typename _Iterator,typename _Check = decltype(std::declval<sharpen::SstVector&>() = *std::declval<_Iterator>()),typename _CheckBlock = sharpen::EnableIf<sharpen::IsSstDataBlock<_Block>::Value>>
        static sharpen::SstRoot MergeTables(sharpen::FileChannelPtr channel,sharpen::Size blockBytes,_Iterator begin,_Iterator end,sharpen::Size filterBits,bool eraseDeleted)
        {
            sharpen::SstRoot root;
            using Maker = sharpen::FileChannelPtr(*)(sharpen::FileChannelPtr);
            Self::MergeTables<_Block>(std::bind(static_cast<Maker>(&Self::GetChannel),channel),blockBytes,0,begin,end,filterBits,eraseDeleted,&root);
            return root;
        }
    };
}

#endif