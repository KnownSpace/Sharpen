#pragma once
#ifndef _SHARPEN_PERSISTENTTABLECONCEPT_HPP
#define _SHARPEN_PERSISTENTTABLECONCEPT_HPP

#include "TypeTraits.hpp"

namespace sharpen
{
    class ByteBuffer;

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
}

#endif