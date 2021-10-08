#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <list>
#include <string>
#include <vector>

using Program = std::vector<int64_t>;
using IO = std::list<int64_t>;
using Phases = std::array<unsigned, 5>;

enum class Ret : int {
    EXIT = 1,
    INPUT = 2,
    OUTPUT = 3,
};

struct Unit {
    Program program;
    unsigned ip = 0;
    IO io;

    Ret run();
};


Ret Unit::run()
{
    for (;;)
    {
        auto s = std::to_string(program.at(ip));
        std::reverse(std::begin(s), std::end(s));
        while(s.size() < 5) {
            s.append("0");
        }

        const auto cmd = s.substr(0, 2);
        bool p1_val = s.at(2) == '1';
        bool p2_val = s.at(3) == '1';

        auto get_param = [this](size_t _ip, bool is_val) {
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
            if (io.empty()) {
                return Ret::INPUT;
            }
            auto& res = program.at(program.at(ip + 1));
            res = io.back();
            io.pop_back();
            ip += 2;
        } else if (cmd == "40") {
            io.push_front(program.at(program.at(ip + 1)));
            ip += 2;
            return Ret::OUTPUT;
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
    return Ret::EXIT;
}

void run_unit(Unit const& unit, Phases phases)
{
    int64_t max_output = 0;
    Phases max_phases;

    do {
        std::vector<Unit> units = {unit, unit, unit, unit, unit};
        assert(phases.size() == units.size());

        // init all with phase setting
        for (size_t i = 0; i < phases.size(); ++i) {
            auto& u = units.at(i);
            u.io.push_back(phases.at(i));
            auto r = u.run();
            assert(r == Ret::INPUT);
        }

        int64_t last_output = 0;
        for (size_t i = 0; i < units.size(); ) {
            auto& u = units.at(i);
            u.io.push_back(last_output);

            auto r = u.run();

            if (r == Ret::OUTPUT) {
                assert(u.io.size() == 1);
                last_output = u.io.front();
                u.io.pop_front();
            } else if (r == Ret::EXIT) {
                assert(u.io.size() == 1);
                last_output = u.io.front();
                u.io.pop_front();
                break;
            }

            if (++i == units.size()) {
                i = 0;
            }
        }

        if (last_output > max_output) {
            max_output = last_output;
            max_phases = phases;
        }
    } while (std::next_permutation(phases.begin(), phases.end()));

    for (auto const& p : max_phases) {
        std::cout << p << ", ";
    };
    std::cout << " with " << max_output << " output\n";
}

int main(int argc, char* argv[])
{
    std::string line;
    std::getline(std::cin, line);

    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 1;
    }

    Unit unit;
    {
        std::vector<std::string> s;
        boost::algorithm::split(s, line, boost::algorithm::is_any_of(","));
        for (auto const& v : s) {
            unit.program.push_back(std::stoi(v));
        }
    }

    std::cout << "1: ";
    run_unit(unit, {0, 1, 2, 3, 4});
    std::cout << "2: ";
    run_unit(unit, {5, 6, 7, 8, 9});

    return 0;
}
