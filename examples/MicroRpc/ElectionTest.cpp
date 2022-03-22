#include <cstdio>
#include <algorithm>
#include <random>
#include <sharpen/RaftWrapper.hpp>
#include <sharpen/RaftMember.hpp>
#include <sharpen/RaftId.hpp>
#include <sharpen/MicroRpcClient.hpp>
#include <sharpen/MicroRpcServer.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/AsyncBarrier.hpp>
#include <sharpen/CtrlHandler.hpp>
#include <sharpen/RandomTimerAdaptor.hpp>
#include <sharpen/Converter.hpp>
#include <sharpen/Console.hpp>
#include <sharpen/Quorum.hpp>

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
    sharpen::Uint64 value_;
public:
    TestLog(sharpen::Uint64 term,sharpen::Uint64 index,sharpen::Uint64 value)
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

    sharpen::Uint64 GetValue() const noexcept
    {
        return this->value_;
    }
};

//pm
class TestPm
{
private:
    std::vector<TestLog> logs_;
    sharpen::Optional<TestId> votedFor_;
    sharpen::Uint64 currentTerm_;
public:

    void AppendLog(const TestLog &log)
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
        return this->votedFor_.Exist();
    }

    void RemoveLog(sharpen::Uint64 index)
    {
        sharpen::Print("remove logs index ",index,"\n");
        auto ite = this->logs_.begin(),end = this->logs_.end();
        if(ite != this->logs_.end())
        {
            this->logs_.erase(++ite,this->logs_.end());
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

    const TestLog &GetLog(sharpen::Uint64 index) const
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

    size_t LogsCount() const
    {
        return this->logs_.size();
    }

    bool EmptyLogs() const
    {
        return this->logs_.empty();
    }

    void AddCurrentTerm()
    {
        this->currentTerm_ += 1;
    }

    void ResetVotedFor()
    {
        this->votedFor_ = sharpen::EmptyOpt;
    }

    bool CheckLog(sharpen::Uint64 index,sharpen::Uint64 expectedTerm) const
    {
        return this->GetLog(index).GetTerm() == expectedTerm;
    }

    sharpen::Uint64 LastLogIndex() const noexcept
    {
        if(this->logs_.empty())
        {
            return 0;
        }
        return this->logs_.back().GetIndex();
    }
};

//commiter
class TestCommiter
{
private:
public:
    static void Commit(const TestLog &log) noexcept
    {
        sharpen::Print("commit log ",log.GetValue(),"\n");
    }
};

//raft state machine
using TestStateMachine = sharpen::RaftWrapper<TestId,TestLog,TestCommiter,TestPm,TestMember>;

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
            try
            {
                auto &&res = client->InvokeAsync(stack);
                if(*res.Top().Data<char>() == 1)
                {
                    sm->ReactVote(1);
                    future.Complete(true);
                    return;
                }
            }
            catch(...){}
            future.Complete(false);
        });
    }

    void Cancel()
    {
        std::printf("cancel\n");
        this->client_->Cancel();
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
            try
            {
                auto &&res = client->InvokeAsync(stack);
                if(*res.Top().Data<char>() == 1)
                {
                    future.Complete(true);
                    return;
                }
            }
            catch(...)
            {}
            future.Complete(false);
        });
    }

    void Cancel()
    {
        std::printf("cancel log\n");
        this->client_->Cancel();
    }
};

bool RaiseElection(TestStateMachine *sm,sharpen::TimerPtr timer)
{
    sm->RaiseElection();
    std::vector<TestElectionProposer> proposers;
    for (auto begin = sm->Members().begin(),end = sm->Members().end();begin != end;++begin)
    {
        try
        {
            sharpen::IpEndPoint addr;
            addr.SetAddrByString("127.0.0.1");
            addr.SetPort(0);
            sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
            conn->Bind(addr);
            addr.SetPort(static_cast<sharpen::UintPort>(begin->second.Id()));
            conn->Register(sharpen::EventEngine::GetEngine());
            conn->ConnectAsync(addr);
            proposers.emplace_back(std::make_shared<sharpen::MicroRpcClient>(conn),sm);   
        }
        catch(...)
        {}
    }
    sharpen::MicroRpcStack req;
    req.Push(sm->LastTerm());
    req.Push(sm->LastIndex());
    req.Push(sm->SelfId());
    req.Push(sm->CurrentTerm());
    const char name[] = "RequestVote";
    req.Push(name,name + sizeof(name) - 1);
    sharpen::AwaitableFuture<bool> continuation;
    sharpen::AwaitableFuture<void> finish;
    std::printf("write request to %zu proposers\n",proposers.size());
    sharpen::Quorum::TimeLimitedProposeAsync(timer,std::chrono::seconds(1),proposers.begin(),proposers.end(),req,continuation,finish);
    std::printf("wait response\n");
    continuation.Await();
    finish.Await();
    std::printf("finish\n");
    return sm->StopElection();
}

void Loop(bool &flag,sharpen::RandomTimerAdaptor &electionTimer,TestStateMachine *sm)
{
    sharpen::Delay(std::chrono::seconds(5));
    while(flag)
    {
        //election
        sharpen::TimerPtr eTimer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        while(sm->GetRole() != sharpen::RaftRole::Leader)
        {
            if(!electionTimer.Await())
            {
                continue;
            }
            sm->ResetLeader();
            sharpen::Print(sm->SelfId()," raise election\n");
            bool success = RaiseElection(sm,eTimer);
            if(success)
            {
                sharpen::Print(sm->SelfId()," became leader term ",sm->CurrentTerm(),"\n");
                break;
            }
            else
            {
                sharpen::Print(sm->SelfId()," election fail\n");
            }
        }
        //log 
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        while(sm->GetRole() == sharpen::RaftRole::Leader)
        {
            //construct loggers
            std::vector<TestlogProposer> loggers;
            for (auto begin = sm->Members().begin(),end = sm->Members().end();begin != end;++begin)
            {
                try
                {
                    sharpen::IpEndPoint addr;
                    addr.SetAddrByString("127.0.0.1");
                    addr.SetPort(0);
                    sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
                    conn->Bind(addr);
                    addr.SetPort(static_cast<sharpen::UintPort>(begin->second.Id()));
                    conn->Register(sharpen::EventEngine::GetEngine());
                    conn->ConnectAsync(addr);
                    loggers.emplace_back(std::make_shared<sharpen::MicroRpcClient>(conn),sm);
                }
                catch(...)
                {}
            }
            //keep alive
            sharpen::MicroRpcStack req;
            req.Clear();
            TestLog log{sm->CurrentTerm(),sm->LastIndex() + 1,sm->LastIndex() + 1};
            //log
            req.Push(log.GetTerm());
            req.Push(log.GetIndex());
            //pre log
            req.Push(sm->LastTerm());
            req.Push(sm->LastIndex());
            //commit index
            req.Push(sm->CommitIndex());
            //leader id
            req.Push(sm->SelfId());
            //current term
            req.Push(sm->CurrentTerm());
            const char name[] = "AppendEntries";
            req.Push(name,name + sizeof(name) - 1);
            sharpen::AwaitableFuture<bool> continuation;
            sharpen::AwaitableFuture<void> finish;
            sharpen::Quorum::TimeLimitedProposeAsync(timer,std::chrono::seconds(1),loggers.begin(),loggers.end(),req,continuation,finish);
            bool success = continuation.Await();
            std::puts("continue log");
            if(!success)
            {
                sharpen::Print(sm->SelfId()," keep alive fail\n");
            }
            else
            {
                sm->AppendLog(log);
                sm->AddCommitIndex(1);
                sm->ApplyLogs();
            }
            finish.Await();
            std::puts("finish log");
            timer->Await(std::chrono::seconds(1));
        }
    }
}

void Test(TestId id,bool *flag,sharpen::MicroRpcServer **ser)
{
    TestStateMachine sm{id,TestPm{}};
    for (size_t i = 0; i < 3; i++)
    {
        if(i + 8080 != id)
        {
            sm.Members().emplace(8080 + i,TestMember{i + 8080});
        }
    }
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(static_cast<sharpen::UintPort>(id));
    {
        char buf[21] = {0};
        addr.GetAddrString(buf,sizeof(buf));
        std::printf("listen on %s:%u\n",buf,addr.GetPort());
    }
    sharpen::MicroRpcServerOption opt{sharpen::MicroRpcDispatcher{}};
    sharpen::MicroRpcServer server{sharpen::AddressFamily::Ip,addr,sharpen::EventEngine::GetEngine(),std::move(opt)};
    sharpen::RandomTimerAdaptor electionTimer{sharpen::EventEngine::GetEngine(),std::chrono::seconds(10),std::chrono::seconds(30),static_cast<sharpen::Uint32>(id ^ std::clock())};
    server.Register("RequestVote",[&sm](sharpen::MicroRpcContext &ctx)
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
            sharpen::Print(sm.SelfId()," vote to ",id,"\n");
        }
        ctx.Connection()->WriteAsync(ctx.Encoder().Encode(stack));
    });
    server.Register("AppendEntries",[&sm,&electionTimer](sharpen::MicroRpcContext &ctx)
    {
        electionTimer.Cancel();
        std::vector<TestLog> logs;
        auto ite = ctx.Request().Begin();
        ++ite;
        sharpen::Uint64 leaderTerm = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 leaderId = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 lastCommit = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 lastIndex = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 lastTerm = *ite->Data<sharpen::Uint64>();
        ++ite;
        sharpen::Uint64 val = *ite->Data<sharpen::Uint64>();
        logs.push_back(TestLog{leaderTerm,val,val});
        bool success = sm.AppendEntries(logs.begin(),logs.end(),leaderId,leaderTerm,lastIndex,lastTerm,lastCommit);
        if(success)
        {
            sharpen::Print(sm.SelfId()," keep alive by ",leaderId,"\n");
        }
        else
        {
            sharpen::Print(sm.SelfId()," keep alive by ",leaderId," but fail\n");
        }
        sharpen::MicroRpcStack stack;
        stack.Push(sm.CurrentTerm());
        stack.Push(static_cast<char>(success));
        ctx.Connection()->WriteAsync(ctx.Encoder().Encode(stack));
    });
    sharpen::Launch(&Loop,*flag,std::ref(electionTimer),&sm);
    *ser = &server;
    std::puts("start server");
    server.RunAsync();
    std::puts("server stop");
}

void Entry(TestId id)
{
    sharpen::StartupNetSupport();
    sharpen::MicroRpcServer *ser{nullptr};
    bool flag = true;
    auto future = sharpen::Async(&Test,id,&flag,&ser);
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[ser,&flag]()
    {
        flag = false;
        ser->Stop();
    });
    future->Await();
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    if(argc == 1)
    {
        std::puts("election test {port}");
        return 0;
    }
    TestId id = sharpen::Atoi<TestId>(argv[1],std::strlen(argv[1]));
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry,id);
    return 0;
}