#pragma once
#ifndef _SHARPEN_RAFTROLE_HPP
#define _SHARPEN_RAFTROLE_HPP

namespace sharpen
{
    enum class RaftRole
    {
        Leader,
        Follower,
        Learner
    };   
}

#endif