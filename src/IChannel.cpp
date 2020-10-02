#include <sharpen/IChannel.hpp>

sharpen::IChannel::~IChannel()
{
    this->Close();
}