#include <cstdio>
#include <algorithm>
#include <random>
#include <sharpen/RaftStateMachine.hpp>
#include <sharpen/RaftMember.hpp>
#include <sharpen/RaftId.hpp>
#include <sharpen/MicroRpcClient.hpp>
#include <sharpen/MicroRpcServer.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/AsyncBarrier.hpp>
#include <sharpen/CtrlHandler.hpp>

//id
using TestId = sharpen::Uint64;

//member
using TestMember = sharpen::RaftMember<TestId>;

//log
class TestLog
{
private:
    using Self = TestLog;

    sharpen::Uint64 term_;
    sharpen::Uint64 index_;
    int value_;
public:
    TestLog(sharpen::Uint64 term,sharpen::Uint64 index,int value)
        :term_(term)
        ,index_(index)
        ,value_(value)
    {}

    TestLog(const Self &other) noexcept = default;

    TestLog(Self &&other) noexcept = default;

    Self &operator=(const Self &other) noexcept = default;

    Self &operator=(Self &&other) noexcept = default;

    ~TestLog() noexcept = default;

    sharpen::Uint64 GetTerm() const noexcept
    {
        return this->term_;
    }

    sharpen::Uint64 GetIndex() const noexcept
    {
        return this->index_;
    }

    void SetTerm(sharpen::Uint64 term) noexcept
    {
        this->term_ = term;
    }

    void SetIndex(sharpen::Uint64 index) noexcept
    {
        this->index_ = index;
    }

    int GetValue() const noexcept
    {
        return this->value_;
    }
};

//pm
class TestPm
{
private:
    std::vector<TestLog> logs_;
    sharpen::Option<TestId> votedFor_;
    sharpen::Uint64 currentTerm_;
public:

    void PushLog(const TestLog &log)
    {
        logs_.push_back(log);
    }

    void SetVotedFor(const TestId &votedFor)
    {
        this->votedFor_.Construct(votedFor);
    }

    void SetCurrentTerm(sharpen::Uint64 term)
    {
        this->currentTerm_ = term;
    }

    sharpen::Uint64 GetCurrentTerm() const
    {
        return this->currentTerm_;
    }

    TestId GetVotedFor() const
    {
        return this->votedFor_.Get();
    }

    bool IsVotedFor() const
    {
        return this->votedFor_.HasValue();
    }

    void EraseLogAfter(sharpen::Uint64 index)
    {
        std::printf("remove logs after index %llu\n",index);
        auto ite = this->logs_.begin(),end = this->logs_.end();
        while (ite != end)
        {
            if (ite->GetIndex() == index)
            {
                break;
            }
            ++ite;
        }
        if(ite != this->logs_.end())
        {
            this->logs_.erase(ite,this->logs_.end());
        }
    }

    bool ContainLog(sharpen::Uint64 index) const
    {
        auto ite = this->logs_.begin(),end = this->logs_.end();
        while (ite != end)
        {
            if (ite->GetIndex() == index)
            {
                break;
            }
            ++ite;
        }
        return ite != this->logs_.end();
    }

    const TestLog &FindLog(sharpen::Uint64 index) const
    {
        auto ite = this->logs_.begin(),end = this->logs_.end();
        while (ite != end)
        {
            if (ite->GetIndex() == index)
            {
                break;
            }
            ++ite;
        }
        return *ite;
    }

    const TestLog &LastLog() const
    {
        return this->logs_.back();
    }

    size_t LogCount() const
    {
        return this->logs_.size();
    }

    bool LogIsEmpty() const
    {
        return this->logs_.empty();
    }

    sharpen::Uint64 AddCurrentTerm()
    {
        return ++this->currentTerm_;
    }

    void ResetVotedFor()
    {
        this->votedFor_ = sharpen::NullOpt;
    }
};

//commiter
class TestCommiter
{
private:
public:
    static void Commit(const TestLog &log) noexcept
    {
        std::printf("commit log value:%d\n",log.GetValue());
    }
};

//raft state machine
using TestStateMachine = sharpen::RaftStateMachine<TestId,TestLog,TestCommiter,TestPm,TestMember>;

//election proposer
class TestElectionProposer
{
private:
    using Self = TestElectionProposer;

    std::shared_ptr<sharpen::MicroRpcClient> client_;
    TestStateMachine *sm_;
public:

    TestElectionProposer(std::shared_ptr<sharpen::MicroRpcClient> client,TestStateMachine *sm)
        :client_(std::move(client))
        ,sm_(sm)
    {}

    TestElectionProposer(const Self &other) = default;

    TestElectionProposer(Self &&other) noexcept = default;

    Self &operator=(const Self &other) = default;

    Self &operator=(Self &&other) noexcept = default;

    ~TestElectionProposer() noexcept = default;

    void ProposeAsync(const sharpen::MicroRpcStack &stack,sharpen::Future<bool> &future)
    {
        std::shared_ptr<sharpen::MicroRpcClient> client = this->client_;
        TestStateMachine *sm = this->sm_;
        sharpen::Launch([&stack,&future,client,sm]()
        {
            auto &&res = client->InvokeAsync(stack);
            if(*res.Top().Data<char>() == 1)
            {
                sm->GetVote(1);
                future.Complete(true);
                return;
            }
            future.Complete(false);
        });
    }
};

//log proposer
class TestlogProposer
{
private:
    using Self = TestlogProposer;

    std::shared_ptr<sharpen::MicroRpcClient> client_;
    TestStateMachine *sm_;
public:

    TestlogProposer(std::shared_ptr<sharpen::MicroRpcClient> client,TestStateMachine *sm)
        :client_(std::move(client))
        ,sm_(sm)
    {}

    TestlogProposer(const Self &other) = default;

    TestlogProposer(Self &&other) noexcept = default;

    Self &operator=(const Self &other) = default;

    Self &operator=(Self &&other) noexcept = default;

    ~TestlogProposer() noexcept = default;

    void ProposeAsync(const sharpen::MicroRpcStack &stack,sharpen::Future<bool> &future)
    {
        std::shared_ptr<sharpen::MicroRpcClient> client = this->client_;
        TestStateMachine *sm = this->sm_;
        sharpen::Launch([&stack,&future,client,sm]()
        {
            auto &&res = client->InvokeAsync(stack);
            if(*res.Top().Data<char>() == 1)
            {
                future.Complete(true);
                return;
            }
            future.Complete(false);
        });
    }
};

void Test(sharpen::UintPort n,sharpen::Size size,sharpen::AsyncBarrier &barrier,std::vector<sharpen::MicroRpcServer*> &servers)
{
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(8080 + n);
    sharpen::MicroRpcServerOption opt{sharpen::MicroRpcDispatcher{}};
    sharpen::MicroRpcServer server(sharpen::AddressFamily::Ip,addr,sharpen::EventEngine::GetEngine(),std::move(opt));
    servers[n] = &server;
    TestStateMachine sm{n,TestPm{}};
    for (size_t i = 0; i < size; i++)
    {
        if(i != n)
        {
            sm.Members().emplace(i,TestMember{i});
        }
    }
    server.Register("RequestVote",[&sm,n](sharpen::MicroRpcContext &ctx)
    {
        auto ite = ctx.Request().Begin();
        ++ite;
        sharpen::Uint64 term = *ite->Data<sharpen::Uint64>();
        ++ite;
        TestId id = *ite->Data<TestId>();
        ++ite;
        sharpen::Uint64 lastLogIndex = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 lastLogTerm = *ite->Data<sharpen::Uint64>();
        bool success = sm.RequestVote(term,id,lastLogIndex,lastLogTerm);
        sharpen::MicroRpcStack stack;
        stack.Push(sm.CurrentTerm());
        stack.Push<char>(success ? 1:0);
        if(success)
        {
            std::printf("%u vote to %llu\n",8080 + n,8080 + id);
        }
        ctx.Connection()->WriteAsync(ctx.Encoder().Encode(stack));
    });
    server.Register("AppendEntries",[&sm,n](sharpen::MicroRpcContext &ctx)
    {
        std::vector<TestLog> logs;
        auto ite = ctx.Request().Begin();
        ++ite;
        sharpen::Uint64 leaderTerm = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 leaderId = *ite->Data<sharpen::Uint64>();
        bool success = sm.AppendEntries(logs.begin(),logs.end(),leaderId,leaderTerm,0,0,0);
        if(success)
        {
            std::printf("%u keep alive by leader %llu\n",8080 + n,8080 + leaderId);
        }
        else
        {
            std::printf("%u keep alive by leader %llu but fail\n",8080 + n,8080 + leaderId);
        }
        sharpen::MicroRpcStack stack;
        stack.Push(sm.CurrentTerm());
        stack.Push(static_cast<char>(success));
        ctx.Connection()->WriteAsync(ctx.Encoder().Encode(stack));
    });
    sharpen::Launch([n,&sm,size]()
    {
        //election
        bool flag = false;
        while(!sm.KnowLeader())
        {
            if (!flag)
            {
                flag = true;
                sharpen::Delay(std::chrono::seconds(12));
            }
            else
            {
                std::mt19937 random(std::clock() + n);
                std::uniform_int_distribution<int> uni(12,25);
                sharpen::Delay(std::chrono::seconds(uni(random)));
            }
            if(sm.KnowLeader())
            {
                std::printf("%u give up election because a known leader\n",n + 8080);
                return;
            }
            std::printf("%u raise election\n",8080 + n);
            sharpen::Uint64 term = sm.RaiseElection();
            std::vector<TestElectionProposer> proposers;
            for (size_t i = 0; i < size; i++)
            {
                if(i != n)
                {
                    sharpen::IpEndPoint addr;
                    addr.SetAddrByString("127.0.0.1");
                    addr.SetPort(0);
                    sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
                    conn->Bind(addr);
                    addr.SetPort(static_cast<sharpen::UintPort>(8080 + i));
                    conn->Register(sharpen::EventEngine::GetEngine());
                    conn->ConnectAsync(addr);
                    proposers.emplace_back(std::make_shared<sharpen::MicroRpcClient>(conn),&sm);
                }   
            }
            sharpen::MicroRpcStack req;
            req.Push(static_cast<sharpen::Uint64>(0));
            req.Push(static_cast<sharpen::Uint64>(0));
            req.Push(sm.SelfId());
            req.Push(sm.CurrentTerm());
            const char name[] = "RequestVote";
            req.Push(name,name + sizeof(name) - 1);
            sharpen::AwaitableFuture<bool> continuation;
            sharpen::AwaitableFuture<void> finish;
            sharpen::Quorum::ProposeAsync(proposers.begin(),proposers.end(),req,continuation,finish);
            bool success = continuation.Await();
            finish.Await();
            if(!sm.StopElection())
            {
                std::printf("%u election fail\n",8080 + n);
                continue;
            }
            std::printf("%u became leader term %llu\n",8080 + n,term);
            break;
        }
        //keep-alive
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        std::vector<TestlogProposer> logers;
        for (size_t i = 0; i < size; i++)
        {
            if(i != n)
            {
                sharpen::IpEndPoint addr;
                addr.SetAddrByString("127.0.0.1");
                addr.SetPort(0);
                sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
                conn->Bind(addr);
                addr.SetPort(static_cast<sharpen::UintPort>(8080 + i));
                conn->Register(sharpen::EventEngine::GetEngine());
                conn->ConnectAsync(addr);
                logers.emplace_back(std::make_shared<sharpen::MicroRpcClient>(conn),&sm);
            }   
        }
        sharpen::MicroRpcStack req;
        while (sm.GetRole() == sharpen::RaftRole::Leader)
        {
            req.Clear();
            req.Push(sm.SelfId());
            req.Push(sm.CurrentTerm());
            const char name[] = "AppendEntries";
            req.Push(name,name + sizeof(name) - 1);
            sharpen::AwaitableFuture<bool> continuation;
            sharpen::AwaitableFuture<void> finish;
            sharpen::Quorum::ProposeAsync(logers.begin(),logers.end(),req,continuation,finish);
            bool success = continuation.Await();
            if(!success)
            {
                std::printf("%u keep alive fail\n",8080 + n);
            }
            finish.Await();
            timer->Await(std::chrono::seconds(10));
        }
    });
    server.RunAsync();
    barrier.Notice();
}

void Entry()
{
    sharpen::StartupNetSupport();
    std::printf("begin election test\n");
    sharpen::AsyncBarrier barrier(3);
    std::vector<sharpen::MicroRpcServer*> servers{3};
    for (size_t i = 0; i < 3; i++)
    {
        sharpen::Launch(&Test,i,3,std::ref(barrier),std::ref(servers));
    }
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[&servers]()
    {
        std::printf("stop now\n");
        for (auto begin = servers.begin(),end = servers.end(); begin != end; ++begin)
        {
            (*begin)->Stop();
        }
    });
    barrier.WaitAsync();
    std::printf("pass\n");
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}