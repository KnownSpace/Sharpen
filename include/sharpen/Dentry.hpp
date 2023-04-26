#pragma once
#ifndef _SHARPEN_DENTRY_HPP
#define _SHARPEN_DENTRY_HPP

#include <utility>
#include <string>

namespace sharpen 
{
    enum class DentryType 
    {
        File,
        Directory
    };

    class Dentry
    {
    private:
        using Self = sharpen::Dentry;
    
        sharpen::DentryType type_;
        std::string path_;
    public:

        explicit Dentry(std::string path) noexcept;
    
        Dentry(sharpen::DentryType type,std::string path) noexcept;
    
        Dentry(const Self &other);
    
        Dentry(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~Dentry() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::DentryType GetType() const noexcept
        {
            return this->type_;
        }

        inline void SetType(sharpen::DentryType type) noexcept
        {
            this->type_ = type;
        }

        inline std::string &Path() noexcept
        {
            return this->path_;
        }
        
        inline const std::string &Path() const noexcept
        {
            return this->path_;
        }
    };
}

#endif