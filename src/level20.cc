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
#include <optional>
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

    Direction()
        : Direction(NONE)
    { }

    Direction(value_t _dx, value_t _dy, value_t _dz) noexcept : dx{_dx}, dy{_dy}, dz{_dz} {}

    value_t get_dx() const noexcept { return dx; }
    value_t get_dy() const noexcept { return dy; }
    value_t get_dz() const noexcept { return dz; }

    bool operator==(const Direction& o) const noexcept { return dx == o.dx && dy == o.dy && dz == o.dz; }

    static Direction oposite(Direction const& d) noexcept
    {
        if (d == Direction::NORTH) return Direction::SOUTH;
        if (d == Direction::SOUTH) return Direction::NORTH;
        if (d == Direction::EAST) return Direction::WEST;
        if (d == Direction::WEST) return Direction::EAST;
        if (d == Direction::UP) return Direction::DOWN;
        if (d == Direction::DOWN) return Direction::UP;

        assert(false);
    }

    static const Direction NONE, NORTH, SOUTH, EAST, WEST, UP, DOWN;
protected:
    value_t dx = 0, dy = 0, dz = 0;
};

const Direction Direction::NONE {0, 0, 0};
const Direction Direction::NORTH {0, -1, 0};
const Direction Direction::SOUTH {0, +1, 0};
const Direction Direction::EAST {+1, 0, 0};
const Direction Direction::WEST {-1, 0, 0};
const Direction Direction::UP {0, 0, 1};
const Direction Direction::DOWN {0, 0, -1};


struct Point
{
    using value_t = int64_t;

    value_t x = -1, y = -1, z = -1;

    Point() = default;

    constexpr Point(value_t x_, value_t y_, value_t z_) noexcept
        : x {x_}
        , y {y_}
        , z {z_}
    { }

    bool operator==(Point const& o) const noexcept
    {
        return x == o.x && y == o.y && z == o.z;
    }

    bool empty() const noexcept { return x == -1 && y == -1 && z == -1; }

    operator bool() const noexcept { return !empty(); }
};

Point operator+(Point const& p, Direction const& d) noexcept
{
    return {p.x + d.get_dx(), p.y + d.get_dy(), p.z + d.get_dz()};
}


struct Tile
{
    unsigned char val {};
    std::optional<Point> dest {};
    std::optional<std::string> dest_name {};
    Direction up_down {};
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
        boost::hash_combine(seed, p.z);
        return seed;
    }
};

std::ostream& operator<<(std::ostream& os, Point const& p)
{
    os << "(" << p.x << "; " << p.y << "; " << p.z << ")";
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
    } else if (d == Direction::UP) {
        os << "UP";
    } else if (d == Direction::DOWN) {
        os << "DOWN";
    } else {
        assert(false);
    }
    return os;
}


}  // namespace std


std::tuple<Point::value_t, Point::value_t, Point::value_t, Point::value_t, Point::value_t, Point::value_t> mapa_min_max(
    Mapa const& mapa)
{
    Point::value_t min_x = std::numeric_limits<Point::value_t>::max(),
                   min_y = std::numeric_limits<Point::value_t>::max(),
                   min_z = std::numeric_limits<Point::value_t>::max(),
                   max_x = std::numeric_limits<Point::value_t>::min(),
                   max_y = std::numeric_limits<Point::value_t>::min(),
                   max_z = std::numeric_limits<Point::value_t>::min();
    for (auto const& [px, _] : mapa) {
        min_x = std::min(min_x, px.x);
        min_y = std::min(min_y, px.y);
        min_z = std::min(min_z, px.z);
        max_x = std::max(max_x, px.x);
        max_y = std::max(max_y, px.y);
        max_z = std::max(max_z, px.z);
    }
    return std::tie(min_x, min_y, min_z, max_x, max_y, max_z);
}


void dump(Mapa const& mapa, Point::value_t z = 0)
{
    std::cout << "-------------------------------------------------------------------------------\n";
    auto const& [min_x, min_y, min_z, max_x, max_y, max_z] = mapa_min_max(mapa);
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x) {
            auto it = mapa.find({x, y, z});
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


Mapa load_map()
{
    Mapa mapa;
    Point p0 {0, 0, 0};

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
    return mapa;
}


void resolve_teleports(Mapa& mapa)
{
    Teleports teleports;

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
            std::swap(p2, p3);  // reading direction
        } else if (check_key(k, Direction::SOUTH)) {
            p2 = p1 + Direction::SOUTH;
            p3 = p2 + Direction::SOUTH;
        } else if (check_key(k, Direction::WEST)) {
            p2 = p1 + Direction::WEST;
            p3 = p2 + Direction::WEST;
            std::swap(p2, p3);  // reading direction
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
            assert(!t.second);
            t.second = p1;
        } else {
            t.first = p1;
        }

        mapa.erase(p2);
        mapa.erase(p3);
        it = mapa.begin();

        // std::cout << "Gate " << g << ", name " << tkey << std::endl;
        // dump(mapa);
    }


    for (auto const& [tkey, gates] : teleports) {
        if (!gates.second) {
            // std::cout << "Teleport " << tkey << " has only one point " << gates.first << std::endl;
            assert(mapa.contains(gates.first));
            mapa.at(gates.first).dest = std::nullopt; // no destination
            mapa.at(gates.first).dest_name = tkey;
        } else {
            assert(mapa.contains(gates.first));
            assert(mapa.contains(gates.second));
            mapa.at(gates.first).dest = gates.second;
            mapa.at(gates.second).dest = gates.first;
            mapa.at(gates.first).dest_name = tkey;
            mapa.at(gates.second).dest_name = tkey;
        }
    }
}


void resolve_outer(Mapa& mapa)
{
    auto const& [min_x, min_y, min_z, max_x, max_y, max_z] = mapa_min_max(mapa);

    // add all verticies
    for (Point::value_t z = min_z; z <= max_z; ++z) {
        for (Point::value_t y = min_y; y <= max_y; ++y) {
            for (Point::value_t x = min_x; x <= max_x; ++x) {
                auto it = mapa.find({x, y, z});
                if (it == mapa.end()) continue;
                it->second.up_down =
                    (x == min_x || x == max_x || y == min_y || y == max_y) ? Direction::UP : Direction::DOWN;
            }
        }
    }
}



Graph create_graph(Mapa& mapa)
{
    Graph g {};

    auto const& [min_x, min_y, min_z, max_x, max_y, max_z] = mapa_min_max(mapa);

    // add all verticies
    for (Point::value_t z = min_z; z <= max_z; ++z) {
        for (Point::value_t y = min_y; y <= max_y; ++y) {
            for (Point::value_t x = min_x; x <= max_x; ++x) {
                const Point px {x, y, z};
                auto it = mapa.find(px);
                if (it == mapa.end() || it->second.val != '.') continue;

                Tile& t = it->second;
                t.vertex = add_vertex(g);
            }
        }
    }

    // connect verticies
    auto link_nodes = [&mapa, &g](Point const& p1, const Direction d) {
        assert(mapa.contains(p1));
        assert(mapa.at(p1).val == '.');
        const Point p2 {p1 + d};
        if (mapa.contains(p2) && mapa.at(p2).val == '.') {
            boost::add_edge(mapa.at(p1).vertex, mapa.at(p2).vertex, EdgeWeightProp {1}, g);
            // std::cout << "Link " << p1 << " with " << p2 << std::endl;
        }
    };

    for (Point::value_t z = min_z; z <= max_z; ++z) {
        for (Point::value_t y = min_y; y <= max_y; ++y) {
            for (Point::value_t x = min_x; x <= max_x; ++x) {
                const Point px {x, y, z};
                const auto it = mapa.find(px);
                if (it == mapa.end() || it->second.val != '.') continue;

                link_nodes(px, Direction::NORTH);
                link_nodes(px, Direction::SOUTH);
                link_nodes(px, Direction::WEST);
                link_nodes(px, Direction::EAST);

                if (it->second.dest) {
                    const Point& p2 = it->second.dest.value();
                    if (mapa.contains(p2)) {  // might go to level not yet added
                        boost::add_edge(mapa.at(px).vertex, mapa.at(p2).vertex, EdgeWeightProp {1}, g);
                        // std::cout << "Link " << px << " with " << p2 << " teleport"<< std::endl;
                    }
                }
            }
        }
    }

    return g;
}

template<typename T>
class TD;

std::pair<Vertex, Vertex> find_start_end(Mapa const& mapa)
{
    Vertex from {}, to {};
    bool has_from {}, has_to {};

    for (auto const& [_, tile] : mapa) {
        if (tile.dest_name) {
            if (!has_from && tile.dest_name.value() == "AA") {
                from = tile.vertex;
                has_from = true;
            } else if (!has_to && tile.dest_name.value() == "ZZ") {
                to = tile.vertex;
                has_to = true;
            }
        }
        if (has_from && has_to) break;
    }

    return {from, to};
}


void part1(Mapa mapa)
{
    auto g = create_graph(mapa);

    const auto& [from, to] = find_start_end(mapa);

    boost::dijkstra_shortest_paths(g, from, boost::distance_map(boost::get(boost::vertex_distance, g)));

    auto const& dist = boost::get(boost::vertex_distance, g);

    std::cout << "1: " << dist[to] << std::endl;
}


void add_map_level(Mapa& mapa, Mapa const& level, Point::value_t const z)
{
    for (auto [p0, tile] : level) {
        Point p1 = p0;
        p1.z = z;
        if (tile.dest) {
            Point& dest = tile.dest.value();
            dest.z = z;
            dest = dest + tile.up_down;
        }
        if (tile.val == '.') {
            mapa.insert({std::move(p1), std::move(tile)});
        }
    }
    // std::cout << "Added level " << z << std::endl;
}


void part2(Mapa mapa)
{
    resolve_outer(mapa);
    Mapa mapa0 = mapa, mapaN = mapa;

    // inner teleports go DOWN; outer go UP

    // close outer teleports on level0
    for (auto& [_, tile] : mapa0) {
        if (tile.dest_name) {
            if (tile.dest_name.value() == "AA" || tile.dest_name.value() == "ZZ") {
                tile.up_down = Direction::NONE;
            } else if (tile.up_down == Direction::UP) {
                tile.val = '#';
                tile.dest = std::nullopt;
                tile.dest_name = std::nullopt;
                tile.up_down = Direction::NONE;
            }
        }
    }

    // close AA and ZZ teleports on levelN
    for (auto& [_, tile] : mapaN) {
        if (tile.dest_name) {
            if (tile.dest_name.value() == "AA" || tile.dest_name.value() == "ZZ") {
                tile.val = '#';
                tile.dest = std::nullopt;
                tile.dest_name = std::nullopt;
                tile.up_down = Direction::NONE;
            }
        }
    }

    Mapa the_map;
    add_map_level(the_map, mapa0, 0);

    for (int i = -1;; --i) {
        add_map_level(the_map, mapaN, i);

        auto g = create_graph(the_map);

        auto const& [from, to] = find_start_end(the_map);

        boost::dijkstra_shortest_paths(g, from, boost::distance_map(boost::get(boost::vertex_distance, g)));

        auto const& dist = boost::get(boost::vertex_distance, g);

        // for (auto const& [p, tile] : the_map) {
        //     if (tile.dest) {
        //         std::cout << "Gate from " << p << " is " << tile.up_down << " to " << tile.dest.value()
        //                   << ", dist: " << dist[tile.vertex] << std::endl;
        //     }
        // }

        if (dist[to] < 4294967295) {
            std::cout << "2: " << dist[to] << std::endl;
            break;
        }
    }
}


int main(int argc, char* argv[])
{
    Mapa mapa = load_map();

    resolve_teleports(mapa);

    part1(mapa);
    part2(mapa);

    return 0;
}
