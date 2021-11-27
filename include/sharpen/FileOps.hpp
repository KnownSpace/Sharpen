#pragma once
#ifndef _SHARPEN_FILEOPS_HPP
#define _SHARPEN_FILEOPS_HPP

#include "FileTypeDef.hpp"

namespace sharpen
{
    bool AccessFile(const char *name,sharpen::FileAccessModel model);

    bool ExistFile(const char *name);

    void RenameFile(const char *oldName,const char *newName);

    void RemoveFile(const char *name);
}

#endif