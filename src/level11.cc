#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using Program = std::map<int64_t, int64_t>;
using IO = std::list<int64_t>;

enum class Ret : int {
    EXIT = 1,
    INPUT = 2,
    OUTPUT = 3,
};

struct Param;

struct Unit {
    int64_t ip = 0;
    int64_t relative_base = 0;

    Program program;
    IO io;

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
            // std::cout << "val: " << get_val(val) << "\n";
            return _u.get_val(val);
        } else if (_mode == '1') {
            // std::cout << "val: " << val << "\n";
            return val;
        } else if (_mode == '2') {
            // std::cout << "val: " << get_val(relative_base + val) << "\n";
            return _u.get_val(_u.relative_base + val);
        } else {
            std::cerr << "UNKNOWN PARAMETER MODE " << _mode << " FOR GET AT " << _ip << std::endl;
            exit(1);
        }
    }

    void set(int64_t value) {
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
        // std::cout << "val: " << get_val(val) << "\n";
        return get_val(val);
    } else if (mode == '1') {
        // std::cout << "val: " << val << "\n";
        return val;
    } else if (mode == '2') {
        // std::cout << "val: " << get_val(relative_base + val) << "\n";
        return get_val(relative_base + val);
    } else {
        std::cerr << "UNKNOWN PARAMETER MODE " << mode << " AT " << ip << std::endl;
        exit(1);
    }
}

void Unit::set_param(int64_t _ip, int64_t value)
{
    program[_ip] = value;
    // std::cout << "@" << _ip << "=" << value << "\n";
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
            Param p1{*this, ip + 1, s.at(2)};
            Param p2{*this, ip + 2, s.at(3)};
            Param p3{*this, ip + 3, s.at(4)};

            // std::cout << "\nIP: " << ip << ", CMD: " << cmd << ", p1_mode: " << p1_mode << ", p2_mode: " << p2_mode
            //           << ", rel: " << relative_base << "\n";

            if (cmd == "99") {
                break;
            } else if (cmd == "10") {
                p3.set(p1() + p2());
                ip += 4;
            } else if (cmd == "20") {
                p3.set(p1() * p2());
                ip += 4;
            } else if (cmd == "30") {
                if (io.empty()) { return Ret::INPUT; }
                p1.set(io.back());
                io.pop_back();
                ip += 2;
            } else if (cmd == "40") {
                io.push_front(p1());
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

struct Point {
    long x = 0, y = 0;

    // Point() = default;
    constexpr Point(long x_, long y_) noexcept
        : x {x_}
        , y {y_}
    { }

    bool operator==(Point const& o) const noexcept { return x == o.x && y == o.y; }

    Point& operator+= (Point const& o) noexcept {
        x += o.x;
        y += o.y;
        return *this;
    }
};

namespace std {

template <>
struct hash<Point>
{
    size_t operator()(Point const& p) const noexcept
    {
        size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        return seed;
    }
};

std::ostream& operator<<(std::ostream& os, Point const& p) {
    os << "(" << p.x << "; " << p.y << ")";
    return os;
}

}  // namespace std

using Mapa = std::unordered_map<Point, unsigned>;

const std::vector<Point> dirs = {
    Point {0, -1},  // up
    Point {1, 0},   // right
    Point {0, 1},   // down
    Point {-1, 0}   // left
};

int64_t run_part1(Unit unit) {
    Mapa mapa;

    int dir = 0;
    Point px {0, 0};
    int outputs = 0;

    for (;;) {
        auto res = unit.run();
        if (res == Ret::EXIT) {
            break;
        } else if (res == Ret::INPUT) {
            unit.io.push_front(mapa[px]);
        } else if (res == Ret::OUTPUT) {
            if (outputs == 0) {
                mapa[px] = unit.io.front();
                unit.io.pop_front();
                outputs = 1;
            } else if (outputs == 1) {
                if (unit.io.front() == 0) {
                    --dir;
                } else {
                    ++dir;
                }
                unit.io.pop_front();
                if (dir < 0) {
                    dir = 3;
                } else if (dir > 3) {
                    dir = 0;
                }
                px += dirs[dir];
                outputs = 0;
            }
        } else {
            assert(false);
        }
    }
    return mapa.size();
}

void run_part2(Unit unit) {
    Mapa mapa;

    int dir = 0;
    Point px {0, 0};
    int outputs = 0;

    mapa[px] = 1; // start on white

    for (;;) {
        auto res = unit.run();
        if (res == Ret::EXIT) {
            break;
        } else if (res == Ret::INPUT) {
            unit.io.push_front(mapa[px]);
        } else if (res == Ret::OUTPUT) {
            if (outputs == 0) {
                mapa[px] = unit.io.front();
                unit.io.pop_front();
                outputs = 1;
            } else if (outputs == 1) {
                if (unit.io.front() == 0) {
                    --dir;
                } else {
                    ++dir;
                }
                unit.io.pop_front();
                if (dir < 0) {
                    dir = 3;
                } else if (dir > 3) {
                    dir = 0;
                }
                px += dirs[dir];
                outputs = 0;
            }
        } else {
            assert(false);
        }
    }

    int64_t min_x = std::numeric_limits<int64_t>::max(), min_y = std::numeric_limits<int64_t>::max(),
            max_x = std::numeric_limits<int64_t>::min(), max_y = std::numeric_limits<int64_t>::min();

    for (auto const& [p, _] : mapa) {
        min_x = std::min(min_x, p.x);
        min_y = std::min(min_y, p.y);
        max_x = std::max(max_x, p.x);
        max_y = std::max(max_y, p.y);
    }

    std::cout << "2:\n";
    for (int64_t y = min_y; y <= max_y; ++y) {
        for (int64_t x = min_x; x <= max_x; ++x) {
            auto col = mapa[{x, y}];
            if (col) {
                std::cout << "X";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "\n";
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

    auto r1 = run_part1(unit);
    std::cout << "1: " << r1 << "\n";

    run_part2(unit);

    return 0;
}
