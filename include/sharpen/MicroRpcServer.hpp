#pragma once
#ifndef _SHARPEN_MICRORPCSERVER_HPP
#define _SHARPEN_MICRORPCSERVER_HPP

#include "RpcServer.hpp"
#include "MicroRpcDecoder.hpp"
#include "MicroRpcEncoder.hpp"
#include "MicroRpcStack.hpp"
#include "MicroRpcDispatcher.hpp"

namespace sharpen
{
    using MicroRpcServer = sharpen::RpcServer<sharpen::MicroRpcStack,sharpen::MicroRpcEncoder,sharpen::MicroRpcStack,sharpen::MicroRpcDecoder,sharpen::MicroRpcDispatcher>;
    using MicroRpcServerOption = typename sharpen::MicroRpcServer::Option;
    using MicroRpcContext = typename sharpen::MicroRpcServer::Context;
}

#endif