#pragma once
#ifndef _SHARPEN_PERSISTENTTABLECONCEPT_HPP
#define _SHARPEN_PERSISTENTTABLECONCEPT_HPP

#include "TypeTraits.hpp"

namespace sharpen
{
    class ByteBuffer;

    class SstBlockHandle;

    template<typename _StoreItem>
    using InternalIsWalStoreItem = auto(*)()->decltype(std::declval<sharpen::ByteBuffer&>() = std::declval<_StoreItem>().Value()
                                                        ,std::declval<bool&>() = std::declval<_StoreItem>().IsDeleted());

    template<typename _StoreItem>
    using IsWalStoreItem = sharpen::IsMatches<sharpen::InternalIsWalStoreItem,_StoreItem>;

    template<typename _Key,typename _Value>
    using InternalIsWalKeyValuePair = auto(*)()->decltype(std::declval<sharpen::ByteBuffer&>() = std::declval<_Key&>()
                                                            ,std::declval<sharpen::InternalIsWalStoreItem<_Value>>());

    template<typename _Kv>
    using IsWalKeyValuePair = sharpen::IsMatches<sharpen::InternalIsWalKeyValuePair,decltype(std::declval<_Kv&>().first),decltype(std::declval<_Kv&>().second)>;

    template<typename _KvIterator>
    using IsWalKeyValuePairIterator = sharpen::IsWalKeyValuePair<decltype(*std::declval<_KvIterator&>())>;

    template<typename _Iterator>
    using InternalIsSstBlockHandleIterator = auto(*)()->decltype(std::declval<sharpen::SstBlockHandle&>() = *std::declval<_Iterator&>());

    template<typename _Iterator>
    using IsSstBlockHandleIterator = sharpen::IsMatches<sharpen::InternalIsSstBlockHandleIterator,_Iterator>;

    template<typename _Block>
    using InternalIsSstDataBlock = auto(*)()->decltype(/*group iterator*/ std::declval<_Block&>().Begin()
                                                        ,/*key iterator*/ std::declval<_Block&>().Begin()->Begin()
                                                        ,/*group iterator*/ std::declval<const _Block&>().Begin()
                                                        ,/*key iterator*/ std::declval<const _Block&>().Begin()->Begin()
                                                        ,std::declval<_Block&>().Put(std::declval<const sharpen::ByteBuffer&>() /*key*/,std::declval<const sharpen::ByteBuffer&>() /*value*/)
                                                        ,std::declval<_Block&>().Delete(std::declval<const sharpen::ByteBuffer&>() /*key*/)
                                                        ,std::declval<_Block&>().EraseDeleted()
                                                        ,std::declval<const _Block&>().Get(std::declval<const sharpen::ByteBuffer&>() /*key*/)
                                                        ,std::declval<_Block&>().LoadFrom(nullptr /*buf*/,0 /*size*/)
                                                        ,std::declval<_Block&>().LoadFrom(std::declval<const sharpen::ByteBuffer&>() /*buf*/,0 /*offset*/)
                                                        ,std::declval<_Block&>().LoadFrom(std::declval<const sharpen::ByteBuffer&>() /*buf*/)
                                                        ,std::declval<sharpen::Size&>() /*writed size*/ = std::declval<const _Block&>().StoreTo(nullptr /*buf*/,0 /*size*/)
                                                        ,std::declval<sharpen::Size&>() /*writed size*/ = std::declval<const _Block&>().StoreTo(std::declval<sharpen::ByteBuffer&>() /*buf*/,0 /*offset*/)
                                                        ,std::declval<sharpen::Size&>() /*writed size*/ = std::declval<const _Block&>().StoreTo(std::declval<sharpen::ByteBuffer&>())
                                                        ,std::declval<bool&>() = std::declval<const _Block&>().Empty()
                                                        ,std::declval<sharpen::Size&>() = std::declval<const _Block&>().GetSize()
                                                        ,std::declval<sharpen::ByteBuffer&>() = std::declval<const _Block&>().LastKey()
                                                        ,std::declval<sharpen::ByteBuffer&>() = std::declval<const _Block&>().FirstKey()
                                                        ,std::declval<_Block&>().Clear()
                                                        ,std::declval<bool&>() = std::declval<const _Block&>().Contain(std::declval<const sharpen::ByteBuffer&>() /*key*/));

    template<typename _Block>
    using IsSstDataBlock = sharpen::IsMatches<sharpen::InternalIsSstDataBlock,_Block>;
}

#endif