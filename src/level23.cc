#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <cassert>
#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using Program = std::map<int64_t, int64_t>;
using IO = std::list<int64_t>;

enum class Ret : int
{
    EXIT = 1,
    INPUT = 2,
    OUTPUT = 3,
};

struct Param;

struct Unit
{
    int64_t ip = 0;
    int64_t relative_base = 0;

    Program program;
    IO input, output;

    Ret run();

protected:
    friend struct Param;

    int64_t get_val(int64_t _ip);
    int64_t get_param(int64_t _ip, char mode);
    void set_param(int64_t _ip, int64_t value);
};

struct Param
{
    Param(Unit& u, int64_t ip, const char mode)
        : _u {u}
        , _ip {ip}
        , _mode {mode}
    { }

    int64_t operator()()
    {
        auto val = _u.get_val(_ip);
        if (_mode == '0') {
            return _u.get_val(val);
        } else if (_mode == '1') {
            return val;
        } else if (_mode == '2') {
            return _u.get_val(_u.relative_base + val);
        } else {
            std::cerr << "UNKNOWN PARAMETER MODE " << _mode << " FOR GET AT " << _ip << std::endl;
            exit(1);
        }
    }

    void set(int64_t value)
    {
        auto idx = _u.get_val(_ip);
        if (_mode == '0') {
            return _u.set_param(idx, value);
        } else if (_mode == '2') {
            return _u.set_param(_u.relative_base + idx, value);
        } else {
            std::cerr << "UNKNOWN PARAMETER MODE " << _mode << " FOR GET AT " << _ip << std::endl;
            exit(1);
        }
    }

private:
    Unit& _u;
    const int64_t _ip;
    const char _mode;
};

// template<typename T> class TD;

int64_t Unit::get_val(int64_t _ip)
{
    auto p = program.insert({_ip, 0LL});
    return p.first->second;
}

int64_t Unit::get_param(int64_t _ip, char mode)
{
    auto val = get_val(_ip);
    if (mode == '0') {
        return get_val(val);
    } else if (mode == '1') {
        return val;
    } else if (mode == '2') {
        return get_val(relative_base + val);
    } else {
        std::cerr << "UNKNOWN PARAMETER MODE " << mode << " AT " << ip << std::endl;
        exit(1);
    }
}

void Unit::set_param(int64_t _ip, int64_t value)
{
    program[_ip] = value;
}

Ret Unit::run()
{
    try {
        for (;;) {
            auto s = std::to_string(program.at(ip));
            std::reverse(std::begin(s), std::end(s));
            while (s.size() < 5) {
                s.append("0");
            }

            const auto cmd = s.substr(0, 2);
            Param p1 {*this, ip + 1, s.at(2)};
            Param p2 {*this, ip + 2, s.at(3)};
            Param p3 {*this, ip + 3, s.at(4)};

            if (cmd == "99") {
                break;
            } else if (cmd == "10") {
                p3.set(p1() + p2());
                ip += 4;
            } else if (cmd == "20") {
                p3.set(p1() * p2());
                ip += 4;
            } else if (cmd == "30") {
                if (input.empty()) {
                    return Ret::INPUT;
                }
                p1.set(input.front());
                input.pop_front();
                ip += 2;
            } else if (cmd == "40") {
                output.push_back(p1());
                ip += 2;
                return Ret::OUTPUT;
            } else if (cmd == "50") {  // jump-if-true
                if (p1()) {
                    ip = p2();
                } else {
                    ip += 3;
                }
            } else if (cmd == "60") {  // jump-if-false
                if (not p1()) {
                    ip = p2();
                } else {
                    ip += 3;
                }
            } else if (cmd == "70") {  // less-than
                p3.set(p1() < p2());
                ip += 4;
            } else if (cmd == "80") {  // equals
                p3.set(p1() == p2());
                ip += 4;
            } else if (cmd == "90") {  // adjust relative_base
                relative_base += p1();
                ip += 2;
            } else {
                std::cerr << "UNKNOWN CMD " << program.at(ip) << " AT " << ip << std::endl;
                exit(1);
            }
        }
        return Ret::EXIT;
    }
    catch (...) {
        std::cerr << "ERROR AT " << ip << std::endl;
        throw;
    }
}


void run1(Unit unit)
{
    std::map<unsigned, Unit> network;
    for (unsigned i = 0; i < 50; ++i) {
        auto [it, _] = network.insert({i, unit});
        it->second.input.push_back(i); // 1st input is the NIC id
    }

    for(;;) {
        for (auto& [id, nic] : network) {
            auto ret = nic.run();
            if (ret == Ret::INPUT) {
                nic.input.push_front(-1);
            } else if (ret == Ret::OUTPUT) {
                const auto dst = nic.output.front();
                nic.output.pop_front();

                ret = nic.run(); // expect X
                assert(ret == Ret::OUTPUT);
                const auto X = nic.output.front();
                nic.output.pop_front();


                ret = nic.run(); // expect Y
                assert(ret == Ret::OUTPUT);
                const auto Y = nic.output.front();
                nic.output.pop_front();

                if (dst == 255) {
                    std::cout << "1: " << Y << std::endl;
                    return;
                }

                assert(dst < 50);
                auto& target = network.at(dst);
                target.input.push_back(X);
                target.input.push_back(Y);
            } else {
                assert(ret == Ret::EXIT);
                std::cout << "END of " << id << " ? " << std::endl;
            }
        }
    }
}


void run2(Unit unit)
{
    std::map<unsigned, Unit> network;
    for (unsigned i = 0; i < 50; ++i) {
        auto [it, _] = network.insert({i, unit});
        it->second.input.push_back(i); // 1st input is the NIC id
    }

    int64_t NAT_X = -1, NAT_Y = -1;
    int64_t NAT_X2 = 0, NAT_Y2 = 0;

    for(;;) {

        unsigned hungry = 0;

        for (auto& [id, nic] : network) {
            auto ret = nic.run();
            if (ret == Ret::INPUT) {
                nic.input.push_front(-1);
                ++hungry;
            } else if (ret == Ret::OUTPUT) {
                const auto dst = nic.output.front();
                nic.output.pop_front();

                ret = nic.run(); // expect X
                assert(ret == Ret::OUTPUT);
                const auto X = nic.output.front();
                nic.output.pop_front();


                ret = nic.run(); // expect Y
                assert(ret == Ret::OUTPUT);
                const auto Y = nic.output.front();
                nic.output.pop_front();

                if (dst == 255) {
                    NAT_X = X;
                    NAT_Y = Y;
                } else {
                    assert(dst < 50);
                    auto& target = network.at(dst);
                    target.input.push_back(X);
                    target.input.push_back(Y);
                }
            } else {
                assert(ret == Ret::EXIT);
                std::cout << "END of " << id << " ? " << std::endl;
            }
        }

        if (hungry == 50) {
            if (NAT_X == NAT_X2 && NAT_Y == NAT_Y2) {
                std::cout << "2: " << NAT_Y << std::endl;
                return;
            }

            auto& target = network.at(0);
            target.input.push_back(NAT_X);
            target.input.push_back(NAT_Y);

            NAT_X2 = NAT_X;
            NAT_Y2 = NAT_Y;
        }
    }
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
        int64_t ip = 0;
        std::vector<std::string> s;
        boost::algorithm::split(s, line, boost::algorithm::is_any_of(","));
        for (auto const& v : s) {
            unit.program.insert({ip++, std::stoll(v)});
        }
    }

    run1(unit);
    run2(unit);


    return 0;
}
