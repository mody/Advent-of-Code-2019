#include <boost/container_hash/hash.hpp>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

constexpr unsigned A = 100;
constexpr unsigned B = A/2;

struct Point {
    long x = -1, y = -1;

    Point() = default;
    Point(long x_, long y_)
        : x {x_}
        , y {y_}
    { }

    bool operator==(Point const& o) const noexcept { return x == o.x && y == o.y; }
};

Point operator+(Point lhs, Point const& rhs) noexcept {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

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
    // os << "(" << p.x << "; " << p.y << ")";
    os << "(" << (p.x-B)/A << "; " << (p.y-B)/A << ")";
    return os;
}

}  // namespace std


using World = std::unordered_set<Point>;
using Line = std::deque<Point>;
using PointSet = std::unordered_set<Point>;

Line plotLine(Point p1, Point const& p2)
{
    Line l;
    const long dx = std::abs(p2.x - p1.x);
    const long sx = p1.x < p2.x ? 1 : -1;
    const long dy = -std::abs(p2.y - p1.y);
    const long sy = p1.y < p2.y? 1 : -1;

    for (int err = dx + dy;;) {
        l.push_back(p1);
        if (p1.x == p2.x && p1.y == p2.y) break;
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            p1.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            p1.y += sy;
        }
    }
    return l;
}

int main()
{
    World world;

    std::string line;
    long y = 0;
    while(std::getline(std::cin, line)) {
        long x = 0;
        for (const char& c : line) {
            if (c == '#') {
                world.insert({x*A+B, y*A+B});
            }
            ++x;
        }
        ++y;
    }

    PointSet top_visible;
    Point top;

    for (auto const& p1 : world) {
        PointSet visible;
        for (auto const& p2 : world) {
            if (p1 == p2) { continue; }
            Line l = plotLine (p1, p2);
            l.pop_front();
            for (auto const& px : l) {
                if (world.count(px)) {
                    visible.insert(px);
                    break;
                }
            }
        }
        if (top_visible.size() < visible.size()) {
            top = p1;
            top_visible = std::move(visible);
        }
    }

    std::cout << "top: " << top << ", count: " << top_visible.size() << "\n";

    return 0;
}
