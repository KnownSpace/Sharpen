#include <cstdio>
#include <vector>
#include <sharpen/Quorum.hpp>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/MicroRpcDecoder.hpp>
#include <sharpen/MicroRpcEncoder.hpp>
#include <sharpen/RpcClient.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/AsyncOps.hpp>

using MicroRpcClient = sharpen::RpcClient<sharpen::MicroRpcStack,sharpen::MicroRpcEncoder,sharpen::MicroRpcStack,sharpen::MicroRpcDecoder>;

struct TestQuorumProposer
{
    std::unique_ptr<MicroRpcClient> client_;

    TestQuorumProposer() = default;

    void ProposeAsync(sharpen::MicroRpcStack stack,sharpen::Future<bool> &future)
    {
        sharpen::Launch([stack,this,&future]()
        {
            std::printf("propose\n");
            try
            {
                auto res = this->client_->InvokeAsync(stack);
                std::printf("complete\n");
                future.Complete(true);
            }
            catch(const std::exception &e)
            {
                std::printf("error:%s\n",e.what());
                future.Complete(false);
                return;
            }
        });
    }

    ~TestQuorumProposer() noexcept = default;
};

void Entry()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    std::vector<TestQuorumProposer> proposers{3};
    for (size_t i = 0; i < 3; i++)
    {
        addr.SetPort(0);
        sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
        conn->Bind(addr);
        conn->Register(engine);
        addr.SetPort(8080 + i);
        conn->ConnectAsync(addr);
        proposers[i].client_.reset(new MicroRpcClient{conn});
    }
    sharpen::MicroRpcStack req;
    char proc[] = "Hello";
    //req.Push(1);
    req.Push(proc,proc + sizeof(proc) - 1);
    sharpen::AwaitableFuture<void> finish;
    sharpen::AwaitableFuture<bool> continuation;
    std::printf("start request\n");
    sharpen::Quorum::ProposeAsync(proposers.begin(),proposers.end(),req,continuation,finish);
    bool status = continuation.Await();
    std::printf("status is %d\n",status);
    finish.Await();
    std::printf("finish\n");
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}
