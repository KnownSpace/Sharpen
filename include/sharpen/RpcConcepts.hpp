#pragma once
#ifndef _SHARPEN_RPCCONCEPTS_HPP
#define _SHARPEN_RPCCONCEPTS_HPP

#include "TypeTraits.hpp"

namespace sharpen
{
    class ByteBuffer;

    //encoder concept
    template<typename _Message,typename _Encoder>
    using InternalIsRpcEncoder = auto(*)() -> decltype(std::declval<sharpen::ByteBuffer&>() = std::declval<_Encoder>().Encode(std::declval<_Message>())
                                                        ,std::declval<sharpen::Size&>() = std::declval<_Encoder>().EncodeTo(std::declval<_Message>(),std::declval<sharpen::ByteBuffer&>())
                                                        ,std::declval<sharpen::Size&>() = std::declval<_Encoder>().EncodeTo(std::declval<_Message>(),nullptr,0)
                                                        ,std::declval<sharpen::Size&>() = std::declval<_Encoder>().EncodeTo(std::declval<_Message>(),std::declval<sharpen::ByteBuffer&>(),0));

    template<typename _Message,typename _Encoder>
    using IsRpcEncoder = sharpen::IsMatches<sharpen::InternalIsRpcEncoder,_Message,_Encoder>;

    //decoder concept
    template<typename _Message,typename _Decoder>
    using InternalIsRpcDecoder = auto(*)() -> decltype(std::declval<sharpen::Size&>() = std::declval<_Decoder>().Decode(nullptr,0)
                                                    ,std::declval<bool&>() = std::declval<_Decoder>().IsCompleted()
                                                    ,std::declval<_Decoder>().SetCompleted(false)
                                                    ,std::declval<_Decoder>().Bind(std::declval<_Message&>())
                                                    ,std::declval<sharpen::Size&>() = std::declval<_Decoder>().Decode(std::declval<const sharpen::ByteBuffer&>(),0)
                                                    ,std::declval<sharpen::Size&>() = std::declval<_Decoder>().Decode(std::declval<const sharpen::ByteBuffer&>()));

    template<typename _Message,typename _Decoder>
    using IsRpcDecoder = sharpen::IsMatches<sharpen::InternalIsRpcDecoder,_Message,_Decoder>;

    template<typename _Message>
    using InternalIsRpcMessage = auto(*)()->decltype(std::declval<_Message&>().Clear());

    template<typename _Message>
    using IsRpcMessage = sharpen::IsMatches<sharpen::InternalIsRpcMessage,_Message>;

    template<typename _Request,typename _Dispatcher>
    using InternalIsRpcDispatcher = auto(*)()->decltype(std::declval<_Dispatcher&>().GetProcedureName(std::declval<_Request&>()/*request*/));

    template<typename _Request,typename _Dispatcher>
    using IsRpcDispatcher = sharpen::IsMatches<sharpen::InternalIsRpcDispatcher,_Request,_Dispatcher>;
}

#endif