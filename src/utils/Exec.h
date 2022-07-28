#pragma once

#include <string>
#include <array>
#include <memory>
#include <stdexcept>

namespace prm
{
    /**
     * Executes command in the windows command line
     *
     * @param cmd Command string
     * @return Command line output
     */
    inline std::string Exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
}