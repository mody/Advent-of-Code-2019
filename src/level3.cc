#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container_hash/hash.hpp>

#include <iostream>
#include <string>
#include <unordered_map>
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

struct Cell {
    unsigned char visited = 0;
    unsigned steps1 = 0;
    unsigned steps2 = 0;
};

using Map = std::unordered_map<Point, Cell>;

template<typename OP>
void UpdateMap(std::string const& line, OP&& op)
{
    std::vector<std::string> cmds;
    boost::algorithm::split(cmds, line, boost::algorithm::is_any_of(","));

    Point me {0, 0};
    unsigned steps = 0;
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
            op(me, ++steps);
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
    UpdateMap(line, [&map](Point const& me, unsigned steps) {
        auto& cell = map[me];
        cell.visited |= 0x1;
        cell.steps1 = steps;
    });

    std::getline(std::cin, line);
    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 2;
    }
    UpdateMap(line, [&map](Point const& me, unsigned steps) {
        auto& cell = map[me];
        cell.visited |= 0x2;
        cell.steps2 = steps;
    });

    Point r1, r2;
    unsigned dist = std::numeric_limits<unsigned>::max();
    unsigned steps = std::numeric_limits<unsigned>::max();

    for (auto const& [p, cell] : map) {
        if (cell.visited != 3) continue;
        unsigned d2 = p.distance({0, 0});
        if (d2 < dist) {
            dist = d2;
            r1 = p;
        }
        unsigned ss = cell.steps1 + cell.steps2;
        if (ss < steps) {
            steps = ss;
            r2 = p;
        }
    }
    std::cout << "1: x: " << r1.x << ", y: " << r1.y << ", dist: " << dist << std::endl;
    std::cout << "2: x: " << r2.x << ", y: " << r2.y << ", steps: " << steps << std::endl;

    return 0;
}
