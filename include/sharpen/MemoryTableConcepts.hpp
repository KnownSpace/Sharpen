#pragma once
#ifndef _SHARPEN_MEMORYTABLECONCEPTS_HPP
#define _SHARPEN_MEMORYTABLECONCEPTS_HPP

#include "TypeTraits.hpp"
#include "WriteBatch.hpp"

namespace sharpen
{
    class ByteBuffer;

    template<typename _Logger>
    using InternalIsMemoryTableLogger = auto(*)()->decltype(std::declval<_Logger&>().GetWriteBatchs().begin()
                                                            ,std::declval<_Logger&>().GetWriteBatchs().end()
                                                            ,std::declval<_Logger&>().Clear()
                                                            ,std::declval<_Logger&>().Log(std::declval<const sharpen::WriteBatch&>()/*write batch*/));

    template<typename _Logger>
    using IsMemoryTableLogger = sharpen::IsMatches<sharpen::InternalIsMemoryTableLogger,_Logger>;
}

#endif