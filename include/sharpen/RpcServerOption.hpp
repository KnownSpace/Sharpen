#pragma once
#ifndef _SHARPEN_RPCSERVEROPTION_HPP
#define _SHARPEN_RPCSERVEROPTION_HPP

#include <chrono>
#include <functional>

#include "Option.hpp"

namespace sharpen
{
    template<typename _Encoder,typename _Decoder,typename _Dispatcher>
    class RpcServerOption
    {
    private:
        using Self = sharpen::RpcServerOption<_Encoder,_Decoder,_Dispatcher>;
        using EncoderBuilder = std::function<_Encoder()>;
        using DecoderBuilder = std::function<_Decoder()>;

        _Dispatcher dispatcher_;
        sharpen::Option<std::chrono::milliseconds> timeout_;
        EncoderBuilder encoderBuilder_;
        DecoderBuilder decoderBuilder_;
        
    public:
        template<typename _Rep,typename _Period>
        RpcServerOption(_Dispatcher dispatcher,std::chrono::duration<_Rep,_Period> timeout)
            :RpcServerOption(std::move(dispatcher),std::move(timeout),EncoderBuilder{},DecoderBuilder{})
        {}

        template<typename _Rep,typename _Period>
        RpcServerOption(_Dispatcher dispatcher,std::chrono::duration<_Rep,_Period> timeout,EncoderBuilder encoderBuilder)
            :RpcServerOption(std::move(dispatcher),std::move(timeout),encoderBuilder,DecoderBuilder{})
        {}

        template<typename _Rep,typename _Period>
        RpcServerOption(_Dispatcher dispatcher,std::chrono::duration<_Rep,_Period> timeout,EncoderBuilder encoderBuilder,DecoderBuilder decoderBuilder)
            :dispatcher_(std::move(dispatcher))
            ,timeout_(std::move(timeout))
            ,encoderBuilder_(std::move(encoderBuilder))
            ,decoderBuilder_(std::move(decoderBuilder))
        {}

        explicit RpcServerOption(_Dispatcher dispatcher)
            :RpcServerOption(std::move(dispatcher),EncoderBuilder{},DecoderBuilder{})
        {}

        RpcServerOption(_Dispatcher dispatcher,EncoderBuilder encoderBuilder)
            :RpcServerOption(std::move(dispatcher),std::move(encoderBuilder),DecoderBuilder{})
        {}

        RpcServerOption(_Dispatcher dispatcher,EncoderBuilder encoderBuilder,DecoderBuilder decoderBuilder)
            :dispatcher_(std::move(dispatcher))
            ,timeout_(sharpen::NullOpt)
            ,encoderBuilder_(std::move(encoderBuilder))
            ,decoderBuilder_(std::move(decoderBuilder))
        {}

        RpcServerOption(const Self &other) = default;

        RpcServerOption(Self &&other) noexcept = default;

        ~RpcServerOption() noexcept = default;

        Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(*this,other);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->dispatcher_ = std::move(other.dispatcher_);
                this->timeout_ = std::move(other.timeout_);
                this->encoderBuilder_ = std::move(other.encoderBuilder_);
                this->decoderBuilder_ = std::move(other.decoderBuilder_);
            }
            return *this;
        }

        _Dispatcher &Dispatcher() noexcept
        {
            return this->dispatcher_;
        }

        EncoderBuilder &GetEncoderBuilder() noexcept
        {
            return this->encoderBuilder_;
        }

        DecoderBuilder &GetDecoderBuilder() noexcept
        {
            return this->decoderBuilder_;
        }

        std::chrono::milliseconds &Timeout() noexcept
        {
            return this->timeout_.Get();
        }

        const _Dispatcher &Dispatcher() const noexcept
        {
            return this->dispatcher_;
        }

        const EncoderBuilder &GetEncoderBuilder() const noexcept
        {
            return this->encoderBuilder_;
        }

        const DecoderBuilder &GetDecoderBuilder() const noexcept
        {
            return this->decoderBuilder_;
        }

        const std::chrono::milliseconds &Timeout() const noexcept
        {
            return this->timeout_.Get();
        }

        bool HasTimeout() const noexcept
        {
            return this->timeout_.HasValue();
        }

        sharpen::Option<std::chrono::milliseconds> &TimeoutOption() noexcept
        {
            return this->timeout_;
        }

        const sharpen::Option<std::chrono::milliseconds> &TimeoutOption() const noexcept
        {
            return this->timeout_;
        }
    };
}

#endif