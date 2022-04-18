#pragma once
#ifndef _SHARPEN_RAFTROLE_HPP
#define _SHARPEN_RAFTROLE_HPP

namespace sharpen
{
    enum class RaftRole
    {
        Follower,
        Leader,
        Candidate,
        Learner
    };
}

#endif