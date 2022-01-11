#include <cstdio>
#include <cstdlib>
#include <sharpen/Quorum.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/AsyncOps.hpp>

struct StatelessProposer
{
    static void ProposeAsync(int propose,sharpen::Future<bool> &future)
    {
        sharpen::Launch([propose,&future]() mutable
        {
            static std::atomic_size_t waiteSec{0};
            size_t wait = waiteSec.fetch_add(1);
            sharpen::Delay(std::chrono::seconds(wait));
            std::printf("proposal %d submit\n",propose);
            future.Complete(true);
        });
    }
};

class StatefulProposer
{
private:
    using Self = StatefulProposer;

    bool success_;
public:
    StatefulProposer(bool success)
        :success_(success)
    {}

    StatefulProposer(const Self &other) noexcept = default;

    StatefulProposer(Self &&other) noexcept = default;

    ~StatefulProposer() noexcept = default;

    Self &operator=(const Self &other) noexcept = default;

    Self &operator=(Self &&other) noexcept = default;

    void ProposeAsync(int propose,sharpen::Future<bool> &future)
    {
        bool success = this->success_;
        sharpen::Launch([propose,&future,success]() mutable
        {
            if(success)
            {
                std::printf("proposal %d submit\n",propose);
                future.Complete(true);
                return;
            }
            future.Fail(std::make_exception_ptr(std::logic_error("we fail")));
        });
    }

    bool Success() const noexcept
    {
        return this->success_;
    }
};

class RandomProposer
{
private:
    using Self = RandomProposer;

    bool success_;
public:
    RandomProposer() noexcept = default;

    RandomProposer(const Self &other) noexcept = default;

    RandomProposer(Self &&other) noexcept = default;

    ~RandomProposer() noexcept = default;

    Self &operator=(const Self &other) noexcept = default;

    Self &operator=(Self &&other) noexcept = default;

    void ProposeAsync(int propose,sharpen::Future<bool> &future)
    {
        sharpen::Launch([propose,&future,this]() mutable
        {
            if(std::rand() % 2)
            {
                std::printf("proposal %d submit\n",propose);
                future.Complete(true);
                this->success_ = true;
                return;
            }
            future.Fail(std::make_exception_ptr(std::logic_error("we fail")));
            this->success_ = false;
        });
    }

    bool Success() const noexcept
    {
        return this->success_;
    }
};

class CancelableProposer
{
private:
    using Self = CancelableProposer;

    sharpen::Future<bool> *future_;
public:
    CancelableProposer() noexcept = default;

    CancelableProposer(Self &&other) noexcept = default;

    ~CancelableProposer() noexcept = default;

    Self &operator=(Self &&other) noexcept = default;

    void ProposeAsync(int propose,sharpen::Future<bool> &future)
    {
        this->future_ = &future;
        std::printf("proposing %d but never finish,please cancel me\n",propose);
    }

    void Cancel()
    {
        std::printf("cancel\n");
        this->future_->Complete(false);
    }
};

void StatelessQuorumTest()
{
    std::printf("stateless quorum\n");
    sharpen::Quorum quorum;
    std::vector<StatelessProposer> proposers{10};
    sharpen::AwaitableFuture<bool> continuation;
    sharpen::AwaitableFuture<void> finish;
    quorum.ProposeAsync(proposers.begin(),proposers.end(),1,continuation,finish);
    bool status = continuation.Await();
    assert(status == true);
    std::printf("continue\n");
    std::printf("status is %d\n",status);
    finish.Await();
    std::printf("finish\n");
}

void StatefulQuorumTest()
{
    std::printf("stateful quorum\n");
    sharpen::Quorum quorum;
    std::vector<StatefulProposer> proposers;
    for (size_t i = 0; i < 10; i++)
    {
        proposers.emplace_back(i % 2);
    }
    sharpen::AwaitableFuture<bool> continuation;
    sharpen::AwaitableFuture<void> finish;
    quorum.ProposeAsync(proposers.begin(),proposers.end(),1,continuation,finish);
    bool status = continuation.Await();
    assert(status == true);
    std::printf("continue\n");
    std::printf("status is %d\n",status);
    finish.Await();
    std::printf("finish\n");
    std::printf("proposer status\n");
    for (size_t i = 0; i < 10; i++)
    {
        std::printf("%zu        %d\n",i,proposers[i].Success());
    }
}

void ErrorQuorumTest()
{
    std::printf("error quorum\n");
    sharpen::Quorum quorum;
    std::vector<StatefulProposer> proposers;
    for (size_t i = 0; i < 10; i++)
    {
        proposers.emplace_back(i > 6);
    }
    sharpen::AwaitableFuture<bool> continuation;
    sharpen::AwaitableFuture<void> finish;
    quorum.ProposeAsync(proposers.begin(),proposers.end(),1,continuation,finish);
    bool status = continuation.Await();
    assert(status == false);
    std::printf("continue\n");
    std::printf("status is %d\n",status);
    finish.Await();
    std::printf("finish\n");
    std::printf("proposer status\n");
    for (size_t i = 0; i < 10; i++)
    {
        std::printf("%zu        %d\n",i,proposers[i].Success());
    }
}

void RandomQuorumTest()
{
    std::printf("random stateful quorum\n");
    sharpen::Quorum quorum;
    std::vector<RandomProposer> proposers;
    for (size_t i = 0; i < 10; i++)
    {
        proposers.emplace_back();
    }
    sharpen::AwaitableFuture<bool> continuation;
    sharpen::AwaitableFuture<void> finish;
    quorum.ProposeAsync(proposers.begin(),proposers.end(),1,continuation,finish);
    bool status = continuation.Await();
    std::printf("continue\n");
    std::printf("status is %d\n",status);
    finish.Await();
    std::printf("finish\n");
    sharpen::Size success{0};
    for (size_t i = 0; i < 10; i++)
    {
        if(proposers[i].Success())
        {
            success += 1;
        }
    }
    assert(status == (success >= 5));
}

void CancelableQuorumTest()
{
    std::printf("time limited quorum\n");
    sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
    std::vector<CancelableProposer> proposers;
    for (size_t i = 0; i < 10; i++)
    {
        proposers.emplace_back();
    }
    sharpen::AwaitableFuture<bool> continuation;
    sharpen::AwaitableFuture<void> finish;
    sharpen::Quorum::TimeLimitedProposeAsync(timer,std::chrono::seconds(1),proposers.begin(),proposers.end(),1,continuation,finish);
    bool status = continuation.Await();
    std::printf("continue\n");
    std::printf("status is %d\n",status);
    finish.Await();
    std::printf("finish\n");
}

void QuorumTest()
{
    StatelessQuorumTest();
    StatefulQuorumTest();
    ErrorQuorumTest();
    RandomQuorumTest();
    CancelableQuorumTest();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&QuorumTest);
    return 0;
}
