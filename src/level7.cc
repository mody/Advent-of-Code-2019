#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using Program = std::vector<int64_t>;
using IO = std::list<unsigned>;

IO run_program(Program program, IO io)
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
            res = io.back();
            io.pop_back();
            ip += 2;
        } else if (cmd == "40") {
            io.push_front(program.at(program.at(ip + 1)));
            ip += 2;
        } else if (cmd == "50") { // jump-if-true
            if (get_param(ip + 1, p1_val)) {
                ip = get_param(ip + 2, p2_val);
            } else {
                ip += 3;
            }
        } else if (cmd == "60") { // jump-if-false
            if (not get_param(ip + 1, p1_val)) {
                ip = get_param(ip + 2, p2_val);
            } else {
                ip += 3;
            }
        } else if (cmd == "70") { // less-than
            auto& res = program.at(program.at(ip + 3));
            res = get_param(ip + 1, p1_val) < get_param(ip + 2, p2_val);
            ip += 4;
        } else if (cmd == "80") { // equals
            auto& res = program.at(program.at(ip + 3));
            res = get_param(ip + 1, p1_val) == get_param(ip + 2, p2_val);
            ip += 4;
        } else {
            std::cerr << "UNKNOWN CMD " << program.at(ip) << " AT " << ip << std::endl;
            exit(1);
        }
    }
    return io;
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

    unsigned max_output = 0;
    std::vector<unsigned> max_phases;

    std::vector<unsigned> phases {0, 1, 2, 3, 4};
    do {
        IO io;
        unsigned last_output = 0;
        for (auto const& phase : phases) {
            io.push_front(phase);
            io.push_front(last_output);
            io = run_program(program, std::move(io));
            assert(io.size() == 1);
            last_output = io.front();
            io.clear();
        }
        if (last_output > max_output) {
            max_output = last_output;
            max_phases = phases;
        }
    } while(std::next_permutation(phases.begin(), phases.end()));

    for (auto const& p : max_phases) {
        std::cout << p << ", ";
    };
    std::cout << " with " << max_output << " output\n";

    return 0;
}
