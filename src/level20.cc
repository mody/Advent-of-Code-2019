#include <boost/config.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

using EdgeWeightProp = boost::property<boost::edge_weight_t, unsigned>;
using Graph = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::bidirectionalS,
    boost::property<boost::vertex_distance_t, unsigned>,
    EdgeWeightProp>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

struct Direction
{
    using value_t = int;

    Direction(value_t _dx, value_t _dy) noexcept : dx{_dx}, dy{_dy} {}

    value_t get_dx() const noexcept { return dx; }
    value_t get_dy() const noexcept { return dy; }

    bool operator==(const Direction& o) const noexcept { return dx == o.dx && dy == o.dy; }

    static Direction oposite(Direction const& d) noexcept
    {
        if (d == Direction::NORTH) return Direction::SOUTH;
        if (d == Direction::SOUTH) return Direction::NORTH;
        if (d == Direction::EAST) return Direction::NORTH;
        if (d == Direction::NORTH) return Direction::EAST;

        assert(false);
    }

    static const Direction NORTH, SOUTH, EAST, WEST;
protected:
    value_t dx = 0, dy = 0;
    };

const Direction Direction::NORTH{0, -1};
const Direction Direction::SOUTH{0, +1};
const Direction Direction::EAST{+1,  0};
const Direction Direction::WEST{-1,  0};

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

    // Point& operator+=(Direction const& d) noexcept
    // {
    //     x += d.get_dx();
    //     y += d.get_dy();
    //     return *this;
    // }

    bool empty() const noexcept { return x == -1 && y == -1; }

    operator bool() const noexcept { return !empty(); }
};

Point operator+(Point const& p, Direction const& d) noexcept
{
    return {p.x + d.get_dx(), p.y + d.get_dy()};
}

struct Tile
{
    unsigned char val {};
    std::optional<Point> teleport {};
    Vertex vertex {};
};


using Mapa = std::unordered_map<Point, Tile>;

using Teleport = std::pair<Point, Point>;
using Teleports = std::unordered_map<std::string, Teleport>;

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

std::tuple<Point::value_t, Point::value_t, Point::value_t, Point::value_t> mapa_min_max(Mapa const& mapa)
{
    Point::value_t min_x = std::numeric_limits<Point::value_t>::max(),
                   min_y = std::numeric_limits<Point::value_t>::max(),
                   max_x = std::numeric_limits<Point::value_t>::min(),
                   max_y = std::numeric_limits<Point::value_t>::min();
    for (auto const& [px, _] : mapa) {
        min_x = std::min(min_x, px.x);
        min_y = std::min(min_y, px.y);
        max_x = std::max(max_x, px.x);
        max_y = std::max(max_y, px.y);
    }
    return std::tie(min_x, min_y, max_x, max_y);
}

void dump(Mapa const& mapa)
{
    std::cout << "-------------------------------------------------------------------------------\n";
    auto const& [min_x, min_y, max_x, max_y] = mapa_min_max(mapa);
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x) {
            auto it = mapa.find({x, y});
            if (it == mapa.end()) {
                std::cout << " ";
                continue;
            }
            std::cout << it->second.val;
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------------------------------------------------------------\n";
}


int main(int argc, char* argv[])
{
    Mapa mapa;
    Teleports teleports;

    Point p0 {0, 0};

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
        for (auto c : line) {
            if (c == ' ') {
                ++p0.x;
                continue;
            }
            mapa.insert({p0, {.val = static_cast<unsigned char>(c)}});
            ++p0.x;
        }
        p0.x = 0;
        ++p0.y;
    }

    // dump(mapa);

    {
        // resolve teleports

        auto check_key = [&mapa](Point const& p1, Direction d) noexcept -> bool {
            const Point p2 {p1 + d};
            return (mapa.contains(p2) && (mapa.at(p2).val >= 'A' && mapa.at(p2).val <= 'Z'));
        };

        for (auto it = mapa.begin(); it != mapa.end();) {
            auto const& [k, v] = *it;

            if (!(v.val == '.')) {
                ++it;
                continue;
            }

            const Point p1 = k;
            Point p2;
            Point p3;

            assert(mapa.at(p1).val == '.');

            if (check_key(k, Direction::NORTH)) {
                p2 = p1 + Direction::NORTH;
                p3 = p2 + Direction::NORTH;
                std::swap(p2, p3); // reading direction
            } else if (check_key(k, Direction::SOUTH)) {
                p2 = p1 + Direction::SOUTH;
                p3 = p2 + Direction::SOUTH;
            } else if (check_key(k, Direction::WEST)) {
                p2 = p1 + Direction::WEST;
                p3 = p2 + Direction::WEST;
                std::swap(p2, p3); // reading direction
            } else if (check_key(k, Direction::EAST)) {
                p2 = p1 + Direction::EAST;
                p3 = p2 + Direction::EAST;
            } else {
                ++it;
                continue;
            }

            assert(mapa.contains(p2));
            assert(mapa.contains(p3));

            std::string tkey;
            tkey += mapa.at(p2).val;
            tkey += mapa.at(p3).val;

            Teleport& t = teleports[tkey];
            if (t.first) {
                assert(t.second.empty());
                t.second = p1;
            } else {
                t.first = p1;
            }

            mapa.erase(p2);
            mapa.erase(p3);
            it = mapa.begin();

            // std::cout << "Teleport at " << p1 << ", name " << tkey << std::endl;
            // dump(mapa);
        }
    }

    for (auto const& [tkey, tpoints] : teleports) {
        if (!tpoints.second) {
            // std::cout << "Teleport " << tkey << " has only one point " << tpoints.first << std::endl;
            continue;
        }

        assert(mapa.contains(tpoints.first));
        assert(mapa.contains(tpoints.second));

        mapa.at(tpoints.first).teleport = tpoints.second;
        mapa.at(tpoints.second).teleport = tpoints.first;
    }


    Graph g{};

    auto const& [min_x, min_y, max_x, max_y] = mapa_min_max(mapa);

    // add all verticies
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x) {
            const Point px{x, y};
            auto it = mapa.find(px);
            if (it == mapa.end() || it->second.val != '.') continue;

            Tile& t = it->second;
            t.vertex = add_vertex(g);
        }
    }

    // connect verticies
    {
        auto link_nodes = [&mapa, &g](Point const& p1, const Direction d) {
            assert(mapa.contains(p1));
            assert(mapa.at(p1).val == '.');
            const Point p2 {p1 + d};
            if (mapa.contains(p2) && mapa.at(p2).val == '.') {
                boost::add_edge(mapa.at(p1).vertex, mapa.at(p2).vertex, EdgeWeightProp {1}, g);
                // std::cout << "Link " << p1 << " with " << p2 << std::endl;
            }
        };

        for (Point::value_t y = min_y; y <= max_y; ++y) {
            for (Point::value_t x = min_x; x <= max_x; ++x) {
                const Point px {x, y};
                const auto it = mapa.find(px);
                if (it == mapa.end() || it->second.val != '.') continue;

                link_nodes(px, Direction::NORTH);
                link_nodes(px, Direction::SOUTH);
                link_nodes(px, Direction::WEST);
                link_nodes(px, Direction::EAST);

                if (it->second.teleport) {
                    const Point& p2 = it->second.teleport.value();
                    assert(mapa.contains(p2));
                    boost::add_edge(mapa.at(px).vertex, mapa.at(p2).vertex, EdgeWeightProp {1}, g);
                    // std::cout << "Link " << px << " with " << p2 << " teleport"<< std::endl;
                }
            }
        }
    }

    assert(teleports.contains("AA"));
    assert(teleports.contains("ZZ"));

    assert(mapa.contains(teleports.at("AA").first));
    assert(mapa.contains(teleports.at("ZZ").first));

    auto from = mapa.at(teleports.at("AA").first).vertex;
    auto to = mapa.at(teleports.at("ZZ").first).vertex;

    boost::dijkstra_shortest_paths(g, from, boost::distance_map(boost::get(boost::vertex_distance, g)));

    auto const& dist = boost::get(boost::vertex_distance, g);

    std::cout << "1: " << dist[to] << std::endl;

    // 469 is too low!
    // 470 is too low!

    // day20.m4
    // 620
    // 7366

    return 0;
}
