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
            Param p1{*this, ip + 1, s.at(2)};
            Param p2{*this, ip + 2, s.at(3)};
            Param p3{*this, ip + 3, s.at(4)};

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
    int64_t x = 0, y = 0;

    // Point() = default;
    constexpr Point(int64_t x_, int64_t y_) noexcept
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
            unsigned tile = 0;
            if (it != mapa.end()) {
                tile = it->second;
            }
            switch(tile) {
            case 4: std::cout << "o"; break;
            case 3: std::cout << "-"; break;
            case 2: std::cout << "B"; break;
            case 1: std::cout << "#"; break;
            case 0:
            default: std::cout << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------------------------------------------------------------\n";
}


int64_t run_part1(Unit unit) {
    Mapa mapa;

    for (;;) {
        auto res = unit.run();
        if (res == Ret::EXIT) {
            break;
        } else if (res == Ret::INPUT) {
            assert(false);
        } else if (res == Ret::OUTPUT) {
            if (unit.io.size() < 3) {
                continue;
            }
            int64_t x = unit.io.back(); unit.io.pop_back();
            int64_t y = unit.io.back(); unit.io.pop_back();
            unsigned tile = static_cast<unsigned>(unit.io.back()); unit.io.pop_back();
            assert(unit.io.empty());
            Point p{x, y};
            if (tile == 0) {
                mapa.erase(p);
            } else {
                mapa[p] = tile;
            }
        } else {
            assert(false);
        }
    }

    int64_t blocks = 0;
    for (auto const& [p, tile] : mapa) {
        if (tile == 2) { // BLOCK
            ++blocks;
        }
    }

    return blocks;
}


int64_t run_part2(Unit unit) {
    Mapa mapa;

    unit.program[0] = 2; // play for free

    int64_t score = 0;

    auto find = [&mapa](unsigned t) -> Point {
        for (auto const& [px, tile] : mapa) {
            if (tile == t) {
                return px;
            }
        }
        return {-1, -1};
    };

    for (;;) {
        auto res = unit.run();
        if (res == Ret::EXIT) {
            break;
        } else if (res == Ret::INPUT) {
            assert(unit.io.empty());
            Point me = find(3);
            Point ball = find(4);
            if (me.x > ball.x) {
                unit.io.push_front(-1); // move left
            } else if (me.x < ball.x) {
                unit.io.push_front(1); // move right
            } else {
                unit.io.push_front(0); // joystick doesnt' move
            }
            // dump(mapa);
            // std::cout << "My move: " << unit.io.front() << "\n";
        } else if (res == Ret::OUTPUT) {
            if (unit.io.size() < 3) {
                continue;
            }
            int64_t x = unit.io.back(); unit.io.pop_back();
            int64_t y = unit.io.back(); unit.io.pop_back();
            int64_t tile = unit.io.back(); unit.io.pop_back();
            assert(unit.io.empty());

            Point p{x, y};
            if (p == Point {-1, 0}) {
                score = tile;
            } else if (tile == 0) {
                mapa.erase(p);
            } else {
                mapa[p] = static_cast<unsigned>(tile);
            }
        } else {
            assert(false);
        }
    }

    return score;
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

    auto r2 = run_part2(unit);
    std::cout << "2: " << r2 << "\n";

    return 0;
}
