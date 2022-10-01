#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <algorithm>
#include <array>
#include <cassert>
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
                if (io.empty()) {
                    return Ret::INPUT;
                }
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

struct Point
{
    int64_t x = 0, y = 0;

    Point() = default;

    Point(int64_t x_, int64_t y_) noexcept
        : x {x_}
        , y {y_}
    { }

    bool operator==(Point const& o) const noexcept { return x == o.x && y == o.y; }
};

namespace std {

template<>
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

std::ostream& operator<<(std::ostream& os, Ret const& r)
{
    switch (r) {
    case Ret::EXIT: os << "EXIT"; break;
    case Ret::INPUT: os << "INPUT"; break;
    case Ret::OUTPUT: os << "OUTPUT"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Point const& p)
{
    os << "(" << p.x << "; " << p.y << ")";
    return os;
}

}  // namespace std

using Mapa = std::unordered_map<Point, unsigned char>;

void dump(Mapa const& mapa)
{
    std::cout << "-------------------------------------------------------------------------------\n";
    int64_t min_x = std::numeric_limits<int64_t>::max(), min_y = std::numeric_limits<int64_t>::max(),
            max_x = std::numeric_limits<int64_t>::min(), max_y = std::numeric_limits<int64_t>::min();
    for (auto const& [px, _] : mapa) {
        min_x = std::min(min_x, px.x);
        min_y = std::min(min_y, px.y);
        max_x = std::max(max_x, px.x);
        max_y = std::max(max_y, px.y);
    }
    for (int64_t y = min_y; y <= max_y; ++y) {
        for (int64_t x = min_x; x <= max_x; ++x) {
            auto it = mapa.find({x, y});
            if (it == mapa.end()) {
                std::cout << " ";
                continue;
            }
            std::cout << it->second;
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------------------------------------------------------------\n";
}


void run1(Unit unit)
{
    Mapa mapa;

    for (unsigned y = 0; y < 50; ++y) {
        for (unsigned x = 0; x < 50; ++x) {
            Unit u2 = unit;
            u2.io.push_back(y);
            u2.io.push_back(x);
            auto res = u2.run();
            assert(res == Ret::OUTPUT);
            const auto out = u2.io.front();
            if (out) {
                mapa.insert({{x, y}, '#'});
            }
        }
    }

    std::cout << "1: " << mapa.size() << std::endl;
}

void run2_a(Unit const& unit, const unsigned RECT_SIZE)
{
    Mapa mapa;

    auto processPoint = [&](Point const& p) -> bool {
        Unit u2 = unit;
        u2.io.push_back(p.y);
        u2.io.push_back(p.x);
        auto res = u2.run();
        assert(res == Ret::OUTPUT);
        const auto out = u2.io.front();
        if (out) {
            mapa.insert({p, '#'});
        }
        return !!out;
    };

    Point p1 {}, p0 {};

    if (processPoint(p1)) {
        p0 = p1;
    }

    for (;;) {
        ++p1.x;
        ++p1.y;
        bool has_first = false;
        for (unsigned y = 0; y <= p1.y; ++y) {
            const Point px{p1.x, y};
            if (processPoint(px)) {
                if (!has_first) {
                    p0 = px;
                    has_first = true;
                } else if (has_first) {
                    // this is the first empty after a beam, we can stop here
                    break;
                }
            }
        }

        const Point px{p0.x - (RECT_SIZE - 1), p0.y + (RECT_SIZE - 1)};

        if (mapa.contains(px)) {
            // for (unsigned y = 0; y < RECT_SIZE; ++y) {
            //     for (unsigned x = 0; x < RECT_SIZE; ++x) {
            //         mapa[{p0.x - x, p0.y + y}] = 'O';
            //     }
            // }
            p0 = {p0.x - (RECT_SIZE - 1), p0.y};
            // mapa.at(p0) = 'X';
            break;
        }
    }

    // dump(mapa);
    std::cout << "2: " << (p0.x * 10000 + p0.y) << ", " << p0 << std::endl;

    // 11340700 WRONG!
    // 11330699 (-1, -1) from the previous, still WRONG!
}


void run2_b(Unit const& unit, const unsigned RECT_SIZE)
{
    Mapa mapa;

    constexpr unsigned MAX_SIZE = 2000;
    // constexpr unsigned MAX_SIZE = 200;

    for (unsigned y = 0; y < MAX_SIZE; ++y) {
        bool beam = false;
        for (unsigned x = 0; x < MAX_SIZE; ++x) {
            Unit u2 = unit;
            u2.io.push_back(y);
            u2.io.push_back(x);
            auto res = u2.run();
            assert(res == Ret::OUTPUT);
            const auto out = u2.io.front();
            if (out) {
                mapa.insert({{x, y}, '#'});
                beam = true;
            } else if (beam) {
                // this is the first empty after a beam, we can stop here
                break;
            }
        }
    }

    for (unsigned x = 0; x < MAX_SIZE; ++x) {
        for (unsigned y = 0; y < MAX_SIZE; ++y) {
            Point p0 {x, y};
            if (!mapa.contains(p0)) {
                continue;
            }
            assert(mapa.at(p0) == '#');

            const Point px {p0.x - (RECT_SIZE - 1), p0.y + (RECT_SIZE - 1)};

            if (mapa.contains(px)) {
                // for (unsigned y = 0; y < RECT_SIZE; ++y) {
                //     for (unsigned x = 0; x < RECT_SIZE; ++x) {
                //         mapa[{p0.x - x, p0.y + y}] = 'O';
                //     }
                // }
                p0 = {p0.x - (RECT_SIZE - 1), p0.y};
                // mapa.at(p0) = 'X';
                // dump(mapa);
                std::cout << "2: " << (p0.x * 10000 + p0.y) << ", " << p0 << std::endl;
                return;
            }
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
    // run2_a(unit, 10);
    run2_b(unit, 100);

    return 0;
}
