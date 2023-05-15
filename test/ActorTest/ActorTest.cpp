#include <sharpen/EventEngine.hpp>
#include <sharpen/TcpHost.hpp>
#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/SimpleHostPipeline.hpp>
#include <LogStep.hpp>

std::unique_ptr<sharpen::IHostPipeline> ConfigPipeline() {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipe->Register<LogStep>();
    return pipe;
}

std::unique_ptr<sharpen::TcpHost> CreateHost(std::uint16_t port) {
    sharpen::IpEndPoint endPoint;
    endPoint.SetAddrByString("127.0.0.1");
    endPoint.SetPort(port);
    sharpen::IpTcpStreamFactory streamFactory{endPoint};
    std::unique_ptr<sharpen::TcpHost> host{new (std::nothrow) sharpen::TcpHost{streamFactory}};
    if (!host) {
        throw std::bad_alloc{};
    }
    host->ConfiguratePipeline(&ConfigPipeline);
    return host;
}

int Entry() {
    return 0;
}

int main() {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupEngine()};
    return engine.StartupWithCode(&Entry);
}