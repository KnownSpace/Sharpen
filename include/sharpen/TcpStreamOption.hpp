#pragma once
#ifndef _SHARPEN_TCPSTREAMOPTION_HPP
#define _SHARPEN_TCPSTREAMOPTION_HPP

#include <utility>

namespace sharpen {
    class TcpStreamOption
    {
    private:
        using Self = sharpen::TcpStreamOption;
    
        bool reuseAddr_;
    public:
    
        TcpStreamOption() noexcept;
    
        TcpStreamOption(const Self &other) noexcept = default;
    
        TcpStreamOption(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other) noexcept
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~TcpStreamOption() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        bool IsEnableReuseAddress() const noexcept;

        void SetReuseAddress(bool reuse) noexcept;

        void EnableReuseAddress() noexcept {
            this->SetReuseAddress(true);
        }

        void DisableReuseAddress() noexcept {
            this->SetReuseAddress(false);
        }

        void EnableReuseAddressInNix() noexcept;
    };
}

#endif