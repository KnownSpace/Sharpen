#include <cassert>
#include <cstdio>
#include <cstring>

#include <sharpen/AlignedAlloc.hpp>
#include <sharpen/Directory.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IFileChannel.hpp>


#include <simpletest/TestRunner.hpp>

class ExistTest : public simpletest::ITypenamedTest<ExistTest> {
private:
    using Self = ExistTest;

public:
    ExistTest() noexcept = default;

    ~ExistTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::Directory dir{"."};
        return this->Assert(dir.Exist(), "Directory . should exists, but it not");
    }
};

class EnumTest : public simpletest::ITypenamedTest<EnumTest> {
private:
    using Self = EnumTest;

public:
    EnumTest() noexcept = default;

    ~EnumTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        const char *name = "./TestDir";
        const char *txtFile = "./TestDir/a.txt";
        const char *logFile = "./TestDir/a.log";
        const char *dirPath = "./TestDir/Dir";
        sharpen::MakeDirectory(name);
        sharpen::MakeDirectory(dirPath);
        sharpen::FileChannelPtr channel{sharpen::OpenFileChannel(
            txtFile, sharpen::FileAccessMethod::All, sharpen::FileOpenMethod::CreateNew)};
        channel = sharpen::OpenFileChannel(
            logFile, sharpen::FileAccessMethod::All, sharpen::FileOpenMethod::CreateNew);
        channel->Close();
        sharpen::Directory dir{name};
        if (!dir.Exist()) {
            return this->Fail("./TestDir doesn't exist");
        }
        sharpen::Dentry entry{dir.GetNextEntry()};
        bool hasLog{false};
        bool hasTxt{false};
        bool hasDir{false};
        while (entry.Valid()) {
            if (entry.GetType() == sharpen::FileEntryType::File) {
                if (entry.Name() == "a.txt") {
                    hasLog = true;
                } else if (entry.Name() == "a.log") {
                    hasTxt = true;
                }
            } else if (entry.Name() == "Dir") {
                hasDir = true;
            }
            entry = dir.GetNextEntry();
        }
        sharpen::RemoveFile(txtFile);
        sharpen::RemoveFile(logFile);
        sharpen::DeleteDirectory(dirPath);
        sharpen::DeleteDirectory(name);
        return this->Assert(hasLog && hasTxt && hasDir, "lost some of files");
    }
};

class IteratorTest : public simpletest::ITypenamedTest<IteratorTest> {
private:
    using Self = IteratorTest;

public:
    IteratorTest() noexcept = default;

    ~IteratorTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        const char *name = "./TestDir";
        const char *txtFile = "./TestDir/a.txt";
        const char *logFile = "./TestDir/a.log";
        const char *dirPath = "./TestDir/Dir";
        sharpen::MakeDirectory(name);
        sharpen::MakeDirectory(dirPath);
        sharpen::FileChannelPtr channel{sharpen::OpenFileChannel(
            txtFile, sharpen::FileAccessMethod::All, sharpen::FileOpenMethod::CreateNew)};
        channel = sharpen::OpenFileChannel(
            logFile, sharpen::FileAccessMethod::All, sharpen::FileOpenMethod::CreateNew);
        channel->Close();
        sharpen::Directory dir{name};
        if (!dir.Exist()) {
            return this->Fail("./TestDir doesn't exist");
        }
        bool hasLog{false};
        bool hasTxt{false};
        bool hasDir{false};
        for (auto begin = dir.Begin(), end = dir.End(); begin != end; ++begin) {
            if (begin->GetType() == sharpen::FileEntryType::File) {
                if (begin->Name() == "a.txt") {
                    hasLog = true;
                } else if (begin->Name() == "a.log") {
                    hasTxt = true;
                }
            } else if (begin->Name() == "Dir") {
                hasDir = true;
            }
        }
        sharpen::RemoveFile(txtFile);
        sharpen::RemoveFile(logFile);
        sharpen::DeleteDirectory(dirPath);
        sharpen::DeleteDirectory(name);
        return this->Assert(hasLog && hasTxt && hasDir, "lost some of files");
    }
};

class RemoveAllTest:public simpletest::ITypenamedTest<RemoveAllTest>
{
private:
    using Self = RemoveAllTest;

public:

    RemoveAllTest() noexcept = default;

    ~RemoveAllTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        const char *name = "./TestDir";
        const char *txtFile = "./TestDir/a.txt";
        const char *logFile = "./TestDir/a.log";
        const char *dirPath = "./TestDir/Dir";
        sharpen::MakeDirectory(name);
        sharpen::MakeDirectory(dirPath);
        sharpen::FileChannelPtr channel{sharpen::OpenFileChannel(
            txtFile, sharpen::FileAccessMethod::All, sharpen::FileOpenMethod::CreateNew)};
        channel = sharpen::OpenFileChannel(
            logFile, sharpen::FileAccessMethod::All, sharpen::FileOpenMethod::CreateNew);
        channel->Close();
        sharpen::Directory dir{name};
        dir.RemoveAll();
        return this->Assert(!dir.Exist(),"failed to remove files");
    }
};

static int Test() {
    simpletest::TestRunner runner;
    runner.Register<ExistTest>();
    runner.Register<EnumTest>();
    runner.Register<IteratorTest>();
    runner.Register<RemoveAllTest>();
    return runner.Run();
}

int main() {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupSingleThreadEngine()};
    return engine.StartupWithCode(&Test);
}