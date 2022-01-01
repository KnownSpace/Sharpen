#pragma once
#ifndef _SHARPEN_MEMORYTABLECONCEPTS_HPP
#define _SHARPEN_MEMORYTABLECONCEPTS_HPP

#include "TypeTraits.hpp"

namespace sharpen
{
    class ByteBuffer;

    template<typename _Logger,typename _Map>
    using InternalIsMemoryTableLogger = auto(*)()->decltype(std::declval<_Logger&>().Restore(std::declval<_Map&>() /*in-memory map*/)
                                                            ,std::declval<_Logger&>().LogPut(std::declval<const sharpen::ByteBuffer&>() /*key*/,std::declval<const sharpen::ByteBuffer&>() /*value*/)
                                                            ,std::declval<_Logger&>().LogDelete(std::declval<const sharpen::ByteBuffer&>() /*key*/)
                                                            ,std::declval<_Logger&>().ClearLogs());

    template<typename _Logger,class _Map>
    using IsMemoryTableLogger = sharpen::IsMatches<sharpen::InternalIsMemoryTableLogger,_Logger,_Map>;
}

#endif