#pragma once
#ifndef _SHARPEN_PROCESS_HPP
#define _SHARPEN_PROCESS_HPP

#include "FileTypeDef.hpp"
#include "IInputPipeChannel.hpp"
#include "IOutputPipeChannel.hpp"
#include "SystemError.hpp" // IWYU pragma: keep
#include "SystemMacro.hpp" // IWYU pragma: keep
#include "TypeTraits.hpp" // IWYU pragma: keep
#include <string>
#include <vector>

namespace sharpen
{
    class Process : public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::Process;

        std::string name_;
        std::string workDirectory_;
        std::vector<std::string> args_;
        sharpen::FileHandle handle_;
        sharpen::FileHandle stdin_;
        sharpen::FileHandle stdout_;
        sharpen::FileHandle stderr_;

        void EnsureArgs();

        void WinStart();

        void NixStart();

        std::int32_t WinJoin();

        std::int32_t NixJoin();

        void WinKill();

        void NixKill();

        void ReleaseHandles() noexcept;

        Process(std::string name, std::string workDirectory, std::vector<std::string> args);

    public:
        explicit Process(std::string name)
            : Self{name, std::string{}}
        {
        }

        Process(std::string name, std::string workDirectory);

        template<typename _Iterator,
                 typename _Check =
                     decltype(std::declval<std::string &>() = *std::declval<_Iterator &>()++)>
        Process(std::string name, _Iterator begin, _Iterator end)
            : Self{std::move(name), std::string{}, std::vector<std::string>{begin, end}}
        {
        }

        template<typename _Iterator,
                 typename _Check =
                     decltype(std::declval<std::string &>() = *std::declval<_Iterator &>()++)>
        Process(std::string name, std::string workDirectory, _Iterator begin, _Iterator end)
            : Self{std::move(name), std::move(workDirectory), std::vector<std::string>{begin, end}}
        {
        }

        Process(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~Process() noexcept;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Start();

        std::int32_t Join();

        void Kill();

        void Suspend();

        void Resume();

        sharpen::OutputPipeChannelPtr RedirectStdin();

        sharpen::InputPipeChannelPtr RedirectStdout();

        sharpen::InputPipeChannelPtr RedirectStderr();

        bool Active() const noexcept;
    };
}   // namespace sharpen

#endif