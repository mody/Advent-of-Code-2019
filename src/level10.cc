#include <algorithm>
#include <boost/container_hash/hash.hpp>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>

constexpr long A = 20;
constexpr long B = A/2;

struct Point {
    long x = -1, y = -1, dist = -1;

    Point() = default;
    Point(long x_, long y_)
        : x {x_}
        , y {y_}
    { }

    bool operator==(Point const& o) const noexcept { return x == o.x && y == o.y; }

    bool operator< (Point const& o) const noexcept { return dist < o.dist; }
};

Point make_point(long x, long y)
{
    return Point {x * A + B, y * A + B};
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


using Line = std::deque<Point>;
using PointSet = std::unordered_set<Point>;

class World : public std::unordered_set<Point>
{
    long m_max_x = 0, m_max_y = 0;
public:
    using std::unordered_set<Point>::unordered_set;

    void set_max_x(long x) { m_max_x = x; }
    void set_max_y(long y) { m_max_y = y; }

    long get_max_x() const { return m_max_x; }
    long get_max_y() const { return m_max_y; }

    void dump(PointSet const& px = {}) const {
        for (long y = 0; y < m_max_y; ++y) {
            for (long x = 0; x < m_max_x; ++x) {
                const Point p{make_point(x, y)};
                if (px.count(p)) {
                    std::cout << "X";
                } else if (count(p)) {
                    std::cout << "#";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
};

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

Point part1(World world) {
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
    return top;
}

Point part2(World world, const Point top) {
    std::map<long, Line> m;

    for (auto p : world) {
        long dx = top.x - p.x;
        long dy = top.y - p.y;
        if (dx == 0 && dy == 0) { continue; }
        // p.dist = std::abs(dx) + std::abs(dy);
        p.dist = std::sqrt(dx * dx + dy * dy);
        double angle = -std::atan2(dx, dy);
        if (angle < 0) {
            angle = 2*3.141592654 + angle;
        }
        long k = static_cast<long>(1000*angle);

        m[k].push_back(std::move(p));
    }
    for (auto& [k, l] : m) {
        std::sort(l.begin(), l.end());
    }

    for (unsigned i = 0; !world.empty(); ) {
        for (auto& [k, l] : m) {
            if (l.empty()) continue;
            Point p{l.front()};
            world.erase(p);
            l.pop_front();
            // std::cout << ++i << " - " << p << "\n";
            if (++i == 200) {
                return p;
            }
        }
    }
    return {};
}

int main()
{
    World world;

    std::string line;
    long y = 0, x = 0;
    while(std::getline(std::cin, line)) {
        x = 0;
        for (const char& c : line) {
            if (c == '#') {
                world.insert(make_point(x, y));
            }
            ++x;
        }
        ++y;
    }
    world.set_max_x(x);
    world.set_max_y(y);

    Point top = part1(world);
    Point p200 = part2(world, top);
    std::cout << "p200: " << ((p200.x-B)/A)*100+((p200.y-B)/A) << "\n";

    return 0;
}
