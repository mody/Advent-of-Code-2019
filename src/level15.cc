#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <cassert>
#include <algorithm>
#include <array>
#include <deque>
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

    // Point() = default;
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

    int64_t distance(Point const& o) const {
        // manhattan distance
        return std::abs(x - o.x) + std::abs(y - o.y);
    }

    Direction move_to(Point const& o) const {
        return {(int)(o.x - x), (int)(o.y - y)};
    }
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
            int tile = -1;
            if (it != mapa.end()) {
                tile = it->second;
            }
            switch (tile) {
            case 4: std::cout << "!"; break;
            case 3: std::cout << " "; break;
            case 2: std::cout << "o"; break;
            case 1: std::cout << "."; break;
            case 0: std::cout << "#"; break;
            default: std::cout << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------------------------------------------------------------\n";
}


void run(Unit unit)
{
    auto make_move = [&unit](Direction const& DIR) -> int {
            auto res = unit.run();
            if (res == Ret::INPUT) {
                unit.io.push_front(DIR.to_command());
            } else {
                assert(false);
            }

            res = unit.run();
            if (res == Ret::OUTPUT) {
                assert(unit.io.size() == 1);
                auto x = unit.io.front();
                unit.io.clear();
                return x;
            }
            assert(false);
    };

    Mapa mapa;

    std::deque<Direction> trail;

    std::deque<Point> work_queue;

    auto me = Point{0, 0};
    mapa.insert({me, 0}).first->second = 4;

    for (bool stop = false; !stop;) {
        // std::cout << "Trying " << me << "\n";
        // dump(mapa);
        // std::cout << std::endl;

        for (auto DIR : {Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST}) {
            me += DIR;
            if (mapa.count(me)) {
                // already visited
                me += DIR.reverse();
                continue;
            }

            auto res = make_move(DIR);
            // std::cout << "at " << me << " is " << res << "\n";
            mapa.insert({me, res});

            switch (res) {
            case 0:  // wall
                me += DIR.reverse(); // we didn't actually move
                continue;
            case 1: // free space
                work_queue.push_front(me);
                break;
            case 2: // hole
                std::cout << "1: " << (trail.size() + 1) << std::endl;
                work_queue.push_front(me);
                break;
            default: assert(false);
            }

            // krok zpet
            DIR = DIR.reverse();
            me += DIR;

            auto bck = make_move(DIR);
            assert(bck == 1);
        }

        if (mapa[me] != 2 && mapa[me] != 4) {
            mapa[me] = 3;
        }

        // std::cout << "work_queue - size: " << work_queue.size() << ", trail size: " << trail.size() << std::endl;
        // for (auto const& x : work_queue) {
        //     std::cout << x << ", ";
        // }
        // std::cout << std::endl;

        if (work_queue.empty()) {
            break;
        }

        for (;!work_queue.empty();) {
            auto nextIt = std::find_if(
                work_queue.begin(), work_queue.end(), [&me](Point const& a) { return a.distance(me) == 1; });
            if (nextIt == work_queue.end()) {
                // std::cout << "No one near\n";
                auto DIR = trail.back().reverse();
                trail.pop_back();
                auto res = make_move(DIR);
                assert(res == 1);
                me += DIR;
                continue;
            }
            auto DIR = me.move_to(*nextIt);
            // std::cout << *nextIt << " is near. Direction to it " << DIR << std::endl;
            assert(DIR.to_command() > 0);
            auto res = make_move(DIR);
            assert(res == 1 || res == 2);
            me += DIR;
            assert(me == *nextIt);
            trail.push_back(std::move(DIR));
            work_queue.erase(nextIt);
            break;
        }
    }

    dump(mapa);
    std::cout << std::endl;

    return;
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
