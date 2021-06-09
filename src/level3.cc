#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <iostream>
#include <string>
#include <vector>

struct Point
{
    int x = 0, y = 0;

    void update(Point const& r) noexcept
    {
        x += r.x;
        y += r.y;
    }

    unsigned distance(Point const& rhs) const noexcept { return abs(x - rhs.x) + abs(y - rhs.y); }
};

bool operator==(Point const& l, Point const& r) noexcept
{
    return l.x == r.x && l.y == r.y;
}

template<>
struct std::hash<Point>
{
    size_t operator()(Point const& p) const noexcept
    {
        size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        return seed;
    }
};

using Map = std::unordered_map<Point, int>;

void UpdateMap(Map& map, std::string const& line, int mask)
{
    std::vector<std::string> cmds;
    boost::algorithm::split(cmds, line, boost::algorithm::is_any_of(","));

    Point me{0,0};
    for (auto const& cmd : cmds) {
        Point diff;
        switch (cmd[0]) {
        case 'U': diff.y = 1; break;
        case 'D': diff.y = -1; break;
        case 'R': diff.x = 1; break;
        case 'L': diff.x = -1; break;
        default: assert(false);
        }

        int num = std::stoi(cmd.substr(1));

        for (int i = 0; i < num; ++i) {
            me.update(diff);
            map[me] |= mask;
        }
    }
}


int main(int argc, char* argv[])
{
    Map map;

    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 1;
    }
    UpdateMap(map, line, 0x1);

    std::getline(std::cin, line);
    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 2;
    }
    UpdateMap(map, line, 0x2);

    Point r1;
    unsigned dist = std::numeric_limits<unsigned>::max();

    for (auto const& [p, num] : map) {
        if (num != 3) continue;
        unsigned d2 = p.distance({0,0});
        std::cout << "x: " << p.x << ", y: " << p.y << " = " << d2 << std::endl;
        if (d2 < dist) {
            dist = d2;
            r1 = p;
        }
    }
    std::cout << "1: x: " << r1.x << ", y: " << r1.y << ", dist: " << dist << std::endl;

    return 0;
}
