#include <boost/config.hpp>
#include <boost/container_hash/hash.hpp>

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>


struct Direction
{
    using value_t = int;

    Direction()
        : Direction(NONE)
    { }

    Direction(value_t _dx, value_t _dy) noexcept : dx{_dx}, dy{_dy} {}

    value_t get_dx() const noexcept { return dx; }
    value_t get_dy() const noexcept { return dy; }

    bool operator==(const Direction& o) const noexcept { return dx == o.dx && dy == o.dy; }

    static Direction oposite(Direction const& d) noexcept
    {
        if (d == Direction::NORTH) return Direction::SOUTH;
        if (d == Direction::SOUTH) return Direction::NORTH;
        if (d == Direction::EAST) return Direction::WEST;
        if (d == Direction::WEST) return Direction::EAST;

        assert(false);
    }

    static const Direction NONE, NORTH, SOUTH, EAST, WEST;
protected:
    value_t dx = 0, dy = 0;
};

const Direction Direction::NONE {0, 0};
const Direction Direction::NORTH {0, -1};
const Direction Direction::SOUTH {0, +1};
const Direction Direction::EAST {+1, 0};
const Direction Direction::WEST {-1, 0};


struct Point
{
    using value_t = int64_t;

    value_t x = -1, y = -1;

    Point() = default;

    constexpr Point(value_t x_, value_t y_) noexcept
        : x {x_}
        , y {y_}
    { }

    bool operator==(Point const& o) const noexcept
    {
        return x == o.x && y == o.y;
    }

    bool empty() const noexcept { return x == -1 && y == -1; }

    operator bool() const noexcept { return !empty(); }
};


Point operator+(Point const& p, Direction const& d) noexcept
{
    return {p.x + d.get_dx(), p.y + d.get_dy()};
}


using Mapa = std::unordered_map<Point, unsigned char>;

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


template<>
struct hash<Mapa>
{
    size_t operator()(Mapa const& m) const noexcept
    {
        size_t seed = 0;
        for (auto const& [p, c] : m) {
            boost::hash_combine(seed, std::hash<Point>()(p));
            boost::hash_combine(seed, c);
        }
        return seed;
    }
};


std::ostream& operator<<(std::ostream& os, Point const& p)
{
    os << "(" << p.x << "; " << p.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, Direction const& d)
{
    if (d == Direction::NONE) {
        os << "NONE";
    } else if (d == Direction::WEST) {
        os << "WEST";
    } else if (d == Direction::EAST) {
        os << "EAST";
    } else if (d == Direction::NORTH) {
        os << "NORTH";
    } else if (d == Direction::SOUTH) {
        os << "SOUTH";
    } else {
        assert(false);
    }
    return os;
}


}  // namespace std

std::tuple<Point::value_t, Point::value_t, Point::value_t, Point::value_t> mapa_min_max(
    Mapa const& mapa)
{
    return {0, 0, 4, 4};
}


void dump(Mapa const& mapa)
{
    std::cout << "-------------------------------------------------------------------------------\n";
    auto const& [min_x, min_y, max_x, max_y] = mapa_min_max(mapa);
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x) {
            auto it = mapa.find({x, y});
            if (it == mapa.end()) {
                std::cout << "!";
                continue;
            }
            std::cout << it->second;
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------------------------------------------------------------\n";
}


Mapa load_map()
{
    Mapa mapa;
    Point p0 {0, 0};

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
        for (auto c : line) {
            mapa.insert({p0, c});
            ++p0.x;
        }
        p0.x = 0;
        ++p0.y;
    }
    return mapa;
}


void part1(Mapa mapa)
{
    std::unordered_set<Mapa> seen;

    seen.insert(mapa);

    for (;;) {
        Mapa m2;
        for (auto const& [px, c] : mapa) {
            const auto north = px + Direction::NORTH;
            const auto south = px + Direction::SOUTH;
            const auto west = px + Direction::WEST;
            const auto east = px + Direction::EAST;

            unsigned around = 0;
            if (mapa.contains(north) && mapa.at(north) == '#') {
                ++around;
            }
            if (mapa.contains(south) && mapa.at(south) == '#') {
                ++around;
            }
            if (mapa.contains(west) && mapa.at(west) == '#') {
                ++around;
            }
            if (mapa.contains(east) && mapa.at(east) == '#') {
                ++around;
            }

            if (c == '.' && (around == 1 || around == 2)) {
                m2[px] = '#';
            } else if (c == '#' && around != 1) {
                m2[px] = '.';
            } else {
                m2[px] = c;
            }
        }

        std::swap(mapa, m2);

        if (!seen.insert(mapa).second)
        {
            break;
        }
    }

    int64_t result = 0, increment = 1;

    auto const& [min_x, min_y, max_x, max_y] = mapa_min_max(mapa);
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x, increment <<= 1) {
            auto it = mapa.find({x, y});
            if (it == mapa.end()) {
                continue;
            }
            if (it->second == '#') {
                result += increment;
            }
        }
    }

    std::cout << "1: " << result << std::endl;
}


int main(int argc, char* argv[])
{
    Mapa mapa = load_map();

    dump(mapa);

    part1(mapa);
    // part2(mapa);

    return 0;
}

