#include <EchoReceiver.hpp>

#include <sharpen/DebugTools.hpp>
#include <sharpen/GenericMail.hpp>

EchoReceiver::EchoReceiver() noexcept 
    :counter_(0)
{}

void EchoReceiver::NviReceive(sharpen::Mail mail, std::uint64_t actorId) {
    (void)actorId;
    sharpen::GenericMail mailWrap{mail};
    sharpen::SyncPuts(mailWrap.ImmutableContent().Data());
    counter_.fetch_add(1,std::memory_order_relaxed);
}