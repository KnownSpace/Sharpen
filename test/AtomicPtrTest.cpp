#include <cstdio>
#include <chrono>

#include <sharpen/AtomicPtr.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/StopWatcher.hpp>

#define PARALLEL_TEST_COUNT static_cast<size_t>(1e8)
//1MB fiber stack
#define LINKLIST_TEST_COUNT static_cast<size_t>(1e4)

static sharpen::AtomicPtr<std::shared_ptr<int>> p{nullptr};

static std::shared_ptr<int> sp{nullptr};

static sharpen::SpinLock lock;

void AtomicPtrTest(size_t i)
{
    //std::printf("%zu\n",i);
    p.Store(std::make_shared<int>(i));
    auto lp = p.load();
    if (lp)
    {
        *lp = 1;
    }
}

void LockPtrTest(size_t i)
{
    {
        std::unique_lock<sharpen::SpinLock> lock_(lock);
        std::shared_ptr<int> lp = std::make_shared<int>(i);
        sp = lp;
    }
    std::shared_ptr<int> lp;
    {
        std::unique_lock<sharpen::SpinLock> lock_(lock);
        lp = sp;
    }
    if (lp)
    {
        *lp = 1;
    }
}

struct TestNode
{
    sharpen::SpinLock lock_;
    std::shared_ptr<TestNode> next_;
    size_t val_;
};

struct AtomicTestNode
{
    sharpen::AtomicPtr<std::shared_ptr<AtomicTestNode>> next_;
    size_t val_;
};


struct TestLinkList
{
    std::shared_ptr<TestNode> first_;
    sharpen::SpinLock firstLock_;

    void Insert(size_t val)
    {
        std::shared_ptr<TestNode> curr,pred;
        {
            std::unique_lock<sharpen::SpinLock> lock(this->firstLock_);
            if (!this->first_)
            {
                this->first_ = std::make_shared<TestNode>();
                this->first_->val_ = val;
                return;
            }
            curr = this->first_;
        }
        curr->lock_.Lock();
        while (curr->next_)
        {
            pred = curr;
            curr = curr->next_;
            pred->lock_.Unlock();
            curr->lock_.Lock();
        }
        std::unique_lock<sharpen::SpinLock> lock(curr->lock_,std::adopt_lock);
        curr->next_ = std::make_shared<TestNode>();
        curr->next_->val_ = val;
    }
};

struct AtomicTestLinkList
{
    sharpen::AtomicPtr<std::shared_ptr<AtomicTestNode>> first_;

    void Insert(size_t val)
    {
        std::shared_ptr<AtomicTestNode> node = std::make_shared<AtomicTestNode>();
        std::shared_ptr<AtomicTestNode> curr{nullptr},next{nullptr};
        bool suc = this->first_.CompareAndSwapStrong(curr,node);
        if(suc)
        {
            return;
        }
        curr = this->first_.load();
        next = curr->next_.load();
        while(1)
        {
            while (next)
            {
                curr = next;
                next = curr->next_.load();
            }
            if(curr->next_.CompareAndSwapWeak(next,node))
            {
                return;
            }
        }
    }
};

static TestLinkList *tlist = nullptr;

static AtomicTestLinkList *alist = nullptr;

void ParallelPush(size_t i)
{
    tlist->Insert(i);
}

void AtomicParallelPush(size_t i)
{
    alist->Insert(i);
}

int main()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup([]()
    {
        sharpen::StopWatcher sw;
         std::printf("atomic ptr test begin\n");
        sw.Begin();
        sharpen::ParallelFor(0,PARALLEL_TEST_COUNT,LockPtrTest);
        sw.Stop();
        std::printf("lock parallel using %d sec\n",sw.Compute()/CLOCKS_PER_SEC);
        sw.Begin();
        sharpen::ParallelFor(0,PARALLEL_TEST_COUNT,AtomicPtrTest);
        sw.Stop();
        std::printf("atomic parallel using %d sec\n",sw.Compute()/CLOCKS_PER_SEC);
        //  {
        //     sw.Begin();
        //     TestLinkList list;
        //     tlist = &list;
        //     sharpen::ParallelFor(0,LINKLIST_TEST_COUNT,5,ParallelPush);
        //     sw.Stop();
        //     std::printf("lock linked list using %d time unit\n",sw.Compute());
        // }
        // {
        //     sw.Begin();
        //     AtomicTestLinkList list;
        //     alist = &list;
        //     sharpen::ParallelFor(0,LINKLIST_TEST_COUNT,5,AtomicParallelPush);
        //     sw.Stop();
        //     std::printf("atomic linked list using %d time unit\n",sw.Compute());
        // }
    });
    return 0;
}
