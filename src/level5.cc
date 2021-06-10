
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using Program = std::vector<unsigned>;

unsigned run_program(Program program)
{
    for (unsigned ip = 0;;)
    {
        auto s = std::to_string(program.at(ip));
        std::reverse(std::begin(s), std::end(s));
        while(s.size() < 5) {
            s.append("0");
        }

        const auto cmd = s.substr(0, 2);
        bool p1_val = s.at(2) == '1';
        bool p2_val = s.at(3) == '1';
        // bool p3_val = s.at(4) == '1';

        auto get_param = [&program](size_t _ip, bool is_val) {
            auto val = program.at(_ip);
            if (is_val) {
                return val;
            }
            return program.at(val);
        };

        if (cmd == "99") {
            break;
        } else if (cmd == "10") {
            auto& res = program.at(program.at(ip + 3));
            res = get_param(ip + 1, p1_val) + get_param(ip + 2, p2_val);
            ip += 4;
        } else if (cmd == "20") {
            auto& res = program.at(program.at(ip + 3));
            res = get_param(ip + 1, p1_val) * get_param(ip + 2, p2_val);
            ip += 4;
        } else if (cmd == "30") {
            auto& res = program.at(program.at(ip + 1));
            res = 1;
            ip += 2;
        } else if (cmd == "40") {
            std::cout << "OUT: " << program.at(program.at(ip + 1)) << std::endl;
            ip += 2;
        } else {
            std::cerr << "UNKNOWN CMD " << program.at(ip) << " AT " << ip << std::endl;
            exit(1);
        }
    }
    return program.at(0);
}

int main(int argc, char* argv[])
{
    std::string line;
    std::getline(std::cin, line);

    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 1;
    }

    Program program;
    {
        std::vector<std::string> s;
        boost::algorithm::split(s, line, boost::algorithm::is_any_of(","));
        for (auto const& v : s) {
            program.push_back(std::stoi(v));
        }
    }

    {
        Program p1 = program;
        run_program(p1);
    }

    return 0;
}
