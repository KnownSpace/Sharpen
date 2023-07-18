#include <sharpen/StringOps.hpp>

bool sharpen::Strings::BeginWith(const std::string &str,const std::string &prefix) noexcept {
    if (str.size() < prefix.size()) {
        return false;
    }
    std::size_t size{prefix.size()};
    for(std::size_t i = 0;i != size;++i)
    {
        if (str[i] != prefix[i]) {
            return false;
        }   
    }
    return true;
}

bool sharpen::Strings::EndWith(const std::string &str,const std::string &suffix) noexcept {
    if (str.size() < suffix.size()) {
        return false;
    }
    std::size_t size{suffix.size()};
    for(std::size_t i = 0;i != size;++i)
    {
        if (str[str.size() - size + i] != suffix[i]) {
            return false;
        }   
    }
    return true;
}