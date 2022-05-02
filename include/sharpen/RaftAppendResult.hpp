#pragma once
#ifndef _SHARPEN_RAFTAPPENDRESULT_HPP
#define _SHARPEN_RAFTAPPENDRESULT_HPP

namespace sharpen
{
    enum class RaftAppendResult
    {
        Success,
        AccessDenied,
        LowerTerm,
        LeakOfLogs
    };   
}

#endif