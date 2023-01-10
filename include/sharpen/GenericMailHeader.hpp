#pragma once
#ifndef _SHARPEN_GENERICMAILHEADER_HPP
#define _SHARPEN_GENERICMAILHEADER_HPP

#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>

namespace sharpen
{
    class GenericMailHeader
    {
    private:
        using Self = sharpen::GenericMailHeader;
    
        static constexpr std::size_t formSize_{16};

        std::uint32_t magic_;
        std::uint32_t contentSize_;
        char form_[formSize_];
    public:
    
        static constexpr std::size_t formSize{formSize_};

        GenericMailHeader() noexcept = default;

        explicit GenericMailHeader(std::uint32_t magic) noexcept;

        GenericMailHeader(std::uint32_t magic,std::uint32_t size) noexcept;
    
        GenericMailHeader(const Self &other) noexcept;
    
        GenericMailHeader(Self &&other) noexcept;
    
        Self &operator=(const Self &other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~GenericMailHeader() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline static constexpr std::size_t GetFormSize() noexcept
        {
            return formSize;
        }

        template<typename _T,typename _Check = typename std::enable_if<std::is_standard_layout<_T>::value && sizeof(_T) == formSize>::type>
        inline _T &Form() noexcept
        {
            _T *ptr{reinterpret_cast<_T*>(this->form_)};
            return *ptr;
        }

        template<typename _T,typename _Check = typename std::enable_if<std::is_standard_layout<_T>::value && sizeof(_T) == formSize>::type>
        inline const _T &Form() const noexcept
        {
            const _T *ptr{reinterpret_cast<const _T*>(this->form_)};
            return *ptr;
        }

        inline std::uint32_t GetContentSize() const noexcept
        {
            return this->contentSize_;
        }

        inline void SetContentSize(std::uint32_t contentSize) noexcept
        {
            this->contentSize_ = contentSize;
        }

        inline std::uint32_t GetMagic() const noexcept
        {
            return this->magic_;
        }

        inline void SetMagic(std::uint32_t magic) noexcept
        {
            this->magic_ = magic;
        }
    };    

    
    static_assert(sizeof(sharpen::GenericMailHeader) == 24,"sizeof(GenericMailHeader) should be 24");
}

#endif