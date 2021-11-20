#pragma once
#ifndef _SHARPEN_MICRORPCDISPATCHER_HPP
#define _SHARPEN_MICRORPCDISPATCHER_HPP

#include <string>

#include <sharpen/MicroRpcStack.hpp>

namespace sharpen
{
    class MicroRpcDispatcher
    {
    private:
    public:

        std::string GetProcedureName(const sharpen::MicroRpcStack &stack)
        {
            const auto &top = stack.Top();
            return {top.Data<char>(),top.GetSize()};
        }
    };
}

#endif