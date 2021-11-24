#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <cassert>
#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <iostream>
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


struct Direction
{
    static const Direction NORTH, SOUTH, EAST, WEST;

    Direction(int _dx, int _dy) : dx{_dx}, dy{_dy} {}

    Direction reverse() const {
        return {-1 * dx, -1 * dy};
    }

    constexpr int to_command() const {
        // north (1), south (2), west (3), and east (4)
        if (dx == -1 && dy == 0) {
            return 1; // north
        } else if (dx == 1 && dy == 0) {
            return 2; // south
        } else if (dx == 0 && dy == -1) {
            return 3; // east
        } else if (dx == 0 && dy == 1) {
            return 4; // west
        }
        assert(false);
        return 0;
    }

    int get_dx() const { return dx; }
    int get_dy() const { return dy; }

protected:
    int dx = 0, dy = 0;
};

const Direction Direction::NORTH{-1, 0};
const Direction Direction::SOUTH{ 1, 0};
const Direction Direction::EAST{ 0,  1};
const Direction Direction::WEST{ 0, -1};

struct Point
{
    int64_t x = 0, y = 0;

    constexpr Point(int64_t x_, int64_t y_) noexcept
        : x {x_}
        , y {y_}
    { }

    bool operator==(Point const& o) const noexcept
    {
        return x == o.x && y == o.y;
    }

    Point& operator+=(Direction const& d) noexcept
    {
        x += d.get_dx();
        y += d.get_dy();
        return *this;
    }

    // int64_t distance(Point const& o) const {
    //     // manhattan distance
    //     return std::abs(x - o.x) + std::abs(y - o.y);
    // }
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

    std::ostream& operator<<(std::ostream& os, Direction const& d)
    {
        os << "<" << d.get_dx() << "; " << d.get_dy() << ">";
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


void run(Unit unit)
{
    Mapa mapa;
    Point me {0, 0}, start {0, 0};

    for(;;) {
        auto res = unit.run();
        if (res == Ret::OUTPUT) {
            assert(unit.io.size() == 1);
            switch ((char)unit.io.front()) {
            case '.':
                ++me.x;
                break;
            case '\xa':
                ++me.y;
                me.x = 0;
                break;
            case '#':
                mapa.insert({me, (char)unit.io.front()});
                ++me.x;
                break;
            case '^':
                mapa.insert({me, (char)unit.io.front()});
                start = me;
                ++me.x;
                break;
            }
            unit.io.clear();
        } else {
            break;
        }
    };

    long sum = 0;

    auto check_neighbours = [&mapa](Point const& me) -> int {
        assert(mapa.at(me) == '#');
        int neighbours = 0;
        for (auto const& dir : {Direction::WEST, Direction::EAST, Direction::NORTH, Direction::SOUTH}) {
            Point x{me};
            x += dir;
            auto it = mapa.find(x);
            if (it == mapa.end()) {
                continue;
            }
            if (it->second == '#') {
                ++neighbours;
            }
        }
        return neighbours;
    };

    dump(mapa);

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
                continue;
            }
            if (it->second == '#' && check_neighbours({x, y}) > 2) {
                sum += x*y;
            }
        }
    }
    std::cout << "1: " << sum << "\n";
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

    run(unit);


    return 0;
}
