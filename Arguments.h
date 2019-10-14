#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class Arguments {
public:
    void ParseArguments(int argc, char **argv);
    void SetOption(const std::string &option);
    const std::string &GetOption(const std::string &option);
    [[nodiscard]] bool HasOption(const std::string &option);
    size_t GetParsedArgumentsCount() const;
private:
    std::vector<std::string> m_options;
    std::unordered_map<std::string, std::string> m_parsedOptions;
};
