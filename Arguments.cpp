#include "Arguments.h"

void Arguments::ParseArguments(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        bool found = false;
        for (const auto &option : m_options) {
            if (option == arg) {
                found = true;
                if (i + 1 < argc) {
                    m_parsedOptions.insert(std::make_pair(option, argv[i + 1]));
                    ++i;
                    break;
                } else {
                    throw std::invalid_argument("Option \"" + arg + "\" is missing value!");
                }
            }
        }
        if (!found) {
            throw std::invalid_argument("Unsupported argument \"" + arg + "\"!");
        }
    }
}

void Arguments::SetOption(const std::string &option) {
    m_options.emplace_back(option);
}

const std::string &Arguments::GetOption(const std::string &option) {
    return m_parsedOptions[option];
}

bool Arguments::HasOption(const std::string &option) {
    return m_parsedOptions.find(option) != m_parsedOptions.end();
}

size_t Arguments::GetParsedArgumentsCount() const {
    return m_parsedOptions.size();
}
