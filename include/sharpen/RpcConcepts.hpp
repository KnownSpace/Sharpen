#pragma once
#ifndef _SHARPEN_RPCCONCEPTS_HPP
#define _SHARPEN_RPCCONCEPTS_HPP

#include "TypeTraits.hpp"

namespace sharpen
{
    class ByteBuffer;

    //encoder concept
    template<typename _Message,typename _Encoder>
    using InternalIsRpcEncoder = auto(*)() -> decltype(sharpen::DeclLvalue<sharpen::ByteBuffer>() = std::declval<_Encoder>().Encode(std::declval<_Message>())
                                                        ,sharpen::DeclLvalue<sharpen::Size>() = std::declval<_Encoder>().EncodeTo(std::declval<_Message>(),sharpen::DeclLvalue<sharpen::ByteBuffer>())
                                                        ,sharpen::DeclLvalue<sharpen::Size>() = std::declval<_Encoder>().EncodeTo(std::declval<_Message>(),nullptr,0)
                                                        ,sharpen::DeclLvalue<sharpen::Size>() = std::declval<_Encoder>().EncodeTo(std::declval<_Message>(),sharpen::DeclLvalue<sharpen::ByteBuffer>(),0));

    template<typename _Message,typename _Encoder>
    using IsRpcEncoder = sharpen::IsMatches<sharpen::InternalIsRpcEncoder,_Message,_Encoder>;

    //decoder concept
    template<typename _Message,typename _Decoder>
    using InternalIsRpcDecoder = auto(*)() -> decltype(sharpen::DeclLvalue<sharpen::Size>() = std::declval<_Decoder>().Decode(nullptr,0)
                                                    ,sharpen::DeclLvalue<bool>() = std::declval<_Decoder>().IsCompleted()
                                                    ,std::declval<_Decoder>().SetCompleted(false)
                                                    ,std::declval<_Decoder>().Bind(sharpen::DeclLvalue<_Message>())
                                                    ,sharpen::DeclLvalue<sharpen::Size>() = std::declval<_Decoder>().Decode(std::declval<const sharpen::ByteBuffer&>(),0)
                                                    ,sharpen::DeclLvalue<sharpen::Size>() = std::declval<_Decoder>().Decode(std::declval<const sharpen::ByteBuffer&>()));

    template<typename _Message,typename _Decoder>
    using IsRpcDecoder = sharpen::IsMatches<sharpen::InternalIsRpcDecoder,_Message,_Decoder>;

    template<typename _Message>
    using InternalIsRpcMessage = auto(*)()->decltype(sharpen::DeclLvalue<_Message>().Clear());

    template<typename _Message>
    using IsRpcMessage = sharpen::IsMatches<sharpen::InternalIsRpcMessage,_Message>;

    template<typename _Request,typename _Dispatcher>
    using InternalIsRpcDispatcher = auto(*)()->decltype(sharpen::DeclLvalue<_Dispatcher>().GetProcedureName(sharpen::DeclLvalue<_Request>()/*request*/));

    template<typename _Request,typename _Dispatcher>
    using IsRpcDispatcher = sharpen::IsMatches<sharpen::InternalIsRpcDispatcher,_Request,_Dispatcher>;
}

#endif