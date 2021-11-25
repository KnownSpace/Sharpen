#pragma once
#ifndef _SHARPEN_MICRORPCCLIENT_HPP
#define _SHARPEN_MICRORPCCLIENT_HPP

#include "RpcClient.hpp"
#include "MicroRpcDecoder.hpp"
#include "MicroRpcEncoder.hpp"
#include "MicroRpcStack.hpp"

namespace sharpen
{
    using MicroRpcClient = sharpen::RpcClient<sharpen::MicroRpcStack,sharpen::MicroRpcEncoder,sharpen::MicroRpcStack,sharpen::MicroRpcDecoder>;
}

#endif