#include <boost/config.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/graph/properties.hpp>
#include <cassert>
#include <cctype>
#include <deque>
#include <iostream>
#include <limits>
#include <regex>
#include <string>
#include <unordered_map>

struct Mapa
{
    using CoordType = int;
    using KeyDistanceMap = std::map<char, unsigned>;

    void append(std::string const& line)
    {
        max_x = line.size();
        max_y += 1;
        for (auto c : line) {
            data.push_back(c);
        }
        auto idx = data.find('@');
        if (idx != std::string::npos) {
            std::tie(me_x, me_y) = index_to_xy(idx);
        }
    }

    void dump() const
    {
        std::string line;
        line.reserve(max_x);
        for (CoordType y = 0; y < max_y; ++y) {
            for (CoordType x = 0; x < max_x; ++x) {
                line += get(x, y);
            }
            std::cout << line << "\n";
            line.clear();
        }
    }

    size_t size() const
    {
        return data.size();
    }

    int keys() const {
        int count = 0;
        for (auto const& c : data) {
            if (c >= 'a' && c <= 'z') {
                ++count;
            }
        }
        return count;
    }

    KeyDistanceMap calculate_distances() const
    {
        CoordType origin = data.find('@');
        assert(origin != data.npos);
        assert(origin == xy_to_index(me_x, me_y));

        using EdgeWeightProp = boost::property<boost::edge_weight_t, unsigned>;
        using Graph = boost::adjacency_list<
            boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_distance_t, unsigned>,
            EdgeWeightProp>;
        using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

        Graph g(data.size());

        auto add_edge = [&](Vertex from, Mapa::CoordType x, Mapa::CoordType y) {
            const auto c = get(x, y);
            if (c == '#' || (c >= 'A' && c <= 'Z')) {
                return;
            };
            boost::add_edge(from, xy_to_index(x, y), EdgeWeightProp{1}, g);
        };

        for (size_t idx = 0; idx < data.size(); ++idx) {
            if (data.at(idx) == '#') {
                // wall
                continue;
            }
            auto const& [x, y] = index_to_xy(idx);
            auto from = boost::vertex(idx, g);

            if (has(x - 1, y)) { add_edge(from, x - 1, y); }
            if (has(x + 1, y)) { add_edge(from, x + 1, y); }

            if (has(x, y - 1)) { add_edge(from, x, y - 1); }
            if (has(x, y + 1)) { add_edge(from, x, y + 1); }
        }

        boost::dijkstra_shortest_paths(
            g, boost::vertex(origin, g), boost::distance_map(boost::get(boost::vertex_distance, g)));

        auto const& dist = boost::get(boost::vertex_distance, g);
        KeyDistanceMap kd;
        for (size_t idx = 0; idx < data.size(); ++idx) {
            auto const& c = data.at(idx);
            if (c >= 'a' && c <= 'z') {
                auto d = dist[boost::vertex(idx, g)];
                if (d < std::numeric_limits<int>::max()) {
                    kd[c] = d;
                }
            }
        }

        return kd;
    }

    bool has(CoordType x, CoordType y) const
    {
        return (x >= 0 && y >= 0 && x < max_x && y < max_y);
    }

    char get(CoordType x, CoordType y) const
    {
        assert(has(x, y));
        return data.at(y * max_x + x);
    }

    CoordType xy_to_index(CoordType x, CoordType y) const
    {
        return (y * max_x + x);
    }

    std::pair<CoordType, CoordType> index_to_xy(CoordType index) const
    {
        CoordType x = index % max_x;
        CoordType y = index / max_x;
        return {x, y};
    }

    void move_me(char k)
    {
        CoordType me = data.find('@');
        assert(me != data.npos);

        CoordType key = data.find(k);
        assert(key != data.npos);

        open_door(k);

        data[me] = '.';
        data[key] = '@';

        std::tie(me_x, me_y) = index_to_xy(key);
    }

    friend size_t hash_value(Mapa const& mapa) {
        size_t seed = 0;
        // boost::hash_combine(seed, mapa.data);
        boost::hash_combine(seed, mapa.me_x);
        boost::hash_combine(seed, mapa.me_y);
        boost::hash_combine(seed, mapa.owned_keys);
        return seed;
    }

    bool operator==(Mapa const& o) const noexcept
    {
        assert(max_x == o.max_x);
        assert(max_y == o.max_y);
        // return data == o.data && owned_keys == o.owned_keys;
        return me_x == o.me_x && me_y == o.me_y && owned_keys == o.owned_keys;
    }

protected:

    void open_door(char k)
    {
        assert(k >= 'a');
        assert(k <= 'z');

        unsigned k_idx = 1 << (k - 'a');
        assert((owned_keys & k_idx) == 0);
        owned_keys |= k_idx;

        CoordType door = data.find(std::toupper(k));
        if (door == data.npos) {
            return;
        }
        data[door] = '.';
    }

    CoordType me_x = 0, me_y = 0;
    CoordType max_x = 0, max_y = 0;
    std::string data;
    unsigned owned_keys = 0;
};

namespace std {

template <>
struct hash<Mapa>
{
    size_t operator()(Mapa const& mapa) const noexcept
    {
        return hash_value(mapa);
    }
};

}  // namespace std

using EdgeWeightProp = boost::property<boost::edge_weight_t, unsigned>;
using Graph = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::directedS,
    boost::property<boost::vertex_distance_t, unsigned>,
    EdgeWeightProp>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

using VertexMap = std::map<std::string, Vertex>;

template<typename>
class TD;

int part1(Mapa mapa)
{
    Graph g;

    std::unordered_map<Mapa, Vertex> states;
    std::deque<Mapa> states_queue;

    Vertex start_v = boost::add_vertex(g);
    states.insert({mapa, start_v});
    states_queue.push_back(mapa);

    Vertex dst = boost::add_vertex(g);

    while(!states_queue.empty()) {
        const Mapa m = states_queue.front();
        states_queue.pop_front();

        const Vertex from = states.at(m);

        // std::cout << "\n";
        // m.dump();

        auto next_moves = m.calculate_distances();
        // std::cout << "next_moves: " << next_moves.size() << std::endl;
        if (next_moves.empty()) {
            // end
            // std::cout << "FINISHED" << std::endl;
            boost::add_edge(from, dst, EdgeWeightProp{0}, g);
            continue;
        }

        for (auto const& [key, dist] : next_moves) {
            Mapa m2 = m;
            m2.move_me(key);
            auto it = states.find(m2);
            if (it == states.end()) {
                Vertex to = boost::add_vertex(g);
                auto const& [_, in] = boost::add_edge(from, to, EdgeWeightProp{dist}, g);
                assert(in);
                states.insert({m2, to});
                states_queue.push_back(std::move(m2));
            } else {
                auto const& [_, in] = boost::add_edge(from, it->second, EdgeWeightProp{dist}, g);
                assert(in);
            }
        }
    }

    std::cout << "States: " << states.size() << std::endl;

    auto dist_map = boost::get(boost::vertex_distance, g);

    boost::dijkstra_shortest_paths(g, start_v, boost::distance_map(dist_map));

    return dist_map[dst];
}

int main()
{
    Mapa mapa;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
        mapa.append(line);
    }

    int r1 = part1(mapa);
    std::cout << "1: " << r1 << "\n";

    return 0;
}
