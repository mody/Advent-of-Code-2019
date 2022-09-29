#include <array>
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

// #define DUMP

using CoordType = int;

struct Player {
    CoordType x {}, y {};

    bool operator== (const Player& oth) const noexcept {
        return std::pair(x, y) == std::pair(oth.x, oth.y);
    }

    bool operator< (const Player& oth) const noexcept {
        return std::pair(x, y) < std::pair(oth.x, oth.y);
    }

    friend size_t hash_value(Player const& p) noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        return seed;
    }
};

struct Mapa
{
    using KeyDistanceMap = std::map<char, unsigned>;
    using PlayerKeyDistanceMap = std::map<unsigned, KeyDistanceMap>;

    void append(std::string const& line)
    {
        max_x = line.size();
        max_y += 1;
        for (auto c : line) {
            data.push_back(c);
        }
        players.clear();
        unsigned pos = 0;
        for (auto idx = data.find('@'); idx != std::string::npos; idx = data.find('@', idx + 1)) {
            auto [x, y] = index_to_xy(idx);
            players.insert({pos++, {x, y}});
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
        std::cout << std::hex << "keys: 0x" << owned_keys << std::dec << "\n";
        for (auto const& [ppos, player] : players) {
            std::cout << "P" << ppos << ": {" << player.x << ", " << player.y << "}\n";
        }
    }

    size_t size() const
    {
        return data.size();
    }

    std::string get_data() const noexcept {
        return data;
    }

    PlayerKeyDistanceMap calculate_distances() const
    {
        using EdgeWeightProp = boost::property<boost::edge_weight_t, unsigned>;
        using Graph = boost::adjacency_list<
            boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_distance_t, unsigned>,
            EdgeWeightProp>;
        using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

        Graph g(data.size());

        auto add_edge = [&](Vertex from, CoordType x, CoordType y) {
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

        PlayerKeyDistanceMap pkd;

        for (auto const& [ppos, p] : players) {
            CoordType origin = xy_to_index(p.x, p.y);
            assert(origin != data.npos);

            auto& kd = pkd[ppos];

            boost::dijkstra_shortest_paths(
                g, boost::vertex(origin, g), boost::distance_map(boost::get(boost::vertex_distance, g)));

            auto const& dist = boost::get(boost::vertex_distance, g);
            for (size_t idx = 0; idx < data.size(); ++idx) {
                auto const& c = data.at(idx);
                if (c >= 'a' && c <= 'z') {
                    auto d = dist[boost::vertex(idx, g)];
                    if (d < std::numeric_limits<int>::max()) {
#ifdef DUMP
                        std::cout << "P: " << ppos << ", dist to " << c << " is " << d << std::endl;
#endif
                        kd[c] = d;
                    }
                }
            }
        }

        return pkd;
    }

    bool has(CoordType x, CoordType y) const noexcept
    {
        return (x >= 0 && y >= 0 && x < max_x && y < max_y);
    }

    char get(CoordType x, CoordType y) const
    {
        assert(has(x, y));
        return data.at(y * max_x + x);
    }

    CoordType xy_to_index(CoordType x, CoordType y) const noexcept
    {
        return (y * max_x + x);
    }

    std::pair<CoordType, CoordType> index_to_xy(CoordType index) const noexcept
    {
        CoordType x = index % max_x;
        CoordType y = index / max_x;
        return {x, y};
    }

    Player get_player(unsigned ppos) const {
        return players.at(ppos);
    }

    std::pair<CoordType, CoordType> get_max() const noexcept { return {max_x, max_y}; }

    void move_player(unsigned ppos, char key)
    {
        CoordType keyPos = data.find(key);
        assert(keyPos != data.npos);

        open_door(key);

        Player& player = players.at(ppos);

        data[xy_to_index(player.x, player.y)] = '.';
        data[keyPos] = '@';

        std::tie(player.x, player.y) = index_to_xy(keyPos);
    }

    friend size_t hash_value(Mapa const& mapa) noexcept {
        size_t seed = 0;
        for (auto const& [ppos, p] : mapa.players) {
            boost::hash_combine(seed, ppos);
            boost::hash_combine(seed, p);
        }
        boost::hash_combine(seed, mapa.owned_keys);
        return seed;
    }

    bool operator==(Mapa const& o) const noexcept
    {
        assert(max_x == o.max_x);
        assert(max_y == o.max_y);
        return owned_keys == o.owned_keys && players == o.players;
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
        assert(door < data.size());
        data[door] = '.';
    }

    std::map<unsigned, Player> players;
    CoordType max_x = 0, max_y = 0;
    std::string data {};
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

#ifdef DUMP
        std::cout << "\n";
        m.dump();
#endif

        auto next_moves = m.calculate_distances();

        unsigned empty_moves = 0;
        for (auto const& [ppos, moves] : next_moves) {
#ifdef DUMP
            std::cout << "moves: " << moves.size() << std::endl;
#endif
            if (moves.empty()) {
                // end
                ++empty_moves;
                continue;
            }

            for (auto const& [key, dist] : moves) {
                Mapa m2 = m;
                m2.move_player(ppos, key);
                auto it = states.find(m2);
                if (it == states.end()) {
                    Vertex to = boost::add_vertex(g);
                    auto const& [_, in] = boost::add_edge(from, to, EdgeWeightProp {dist}, g);
                    assert(in);
                    states.insert({m2, to});
                    states_queue.push_back(std::move(m2));
                } else {
#ifdef DUMP
                    std::cout << "state already seen; new dist " << dist << "\n";
#endif
                    auto const& [_, in] = boost::add_edge(from, it->second, EdgeWeightProp {dist}, g);
                    assert(in);
                }
            }
        }

        if (empty_moves == next_moves.size()) {
#ifdef DUMP
            std::cout << "FINISHED" << std::endl;
#endif
            boost::add_edge(from, dst, EdgeWeightProp {0}, g);
        }
    }

    std::cout << "States: " << states.size() << std::endl;

    auto dist_map = boost::get(boost::vertex_distance, g);

    boost::dijkstra_shortest_paths(g, start_v, boost::distance_map(dist_map));

    return dist_map[dst];
}

unsigned part2(Mapa const& m1)
{
    std::string data = m1.get_data();
    auto const& [me_x, me_y] = m1.get_player(0);
    auto const& [max_x, max_y] = m1.get_max();

    assert(me_x == (max_x / 2));
    assert(me_y == (max_y / 2));

    assert(me_x > 1 && me_x + 1 < max_x);
    assert(me_y > 1 && me_y + 1 < max_y);

    data[m1.xy_to_index(me_x, me_y)] = '#';
    data[m1.xy_to_index(me_x - 1, me_y)] = '#';
    data[m1.xy_to_index(me_x + 1, me_y)] = '#';
    data[m1.xy_to_index(me_x, me_y - 1)] = '#';
    data[m1.xy_to_index(me_x, me_y + 1)] = '#';

    data[m1.xy_to_index(me_x - 1, me_y - 1)] = '@';
    data[m1.xy_to_index(me_x + 1, me_y + 1)] = '@';
    data[m1.xy_to_index(me_x + 1, me_y - 1)] = '@';
    data[m1.xy_to_index(me_x - 1, me_y + 1)] = '@';

    Mapa m2;
    for (unsigned y = 0; y < max_y; ++y) {
        m2.append(data.substr(y * max_x, max_x));
    }
    // m1.dump();
    // m2.dump();

    return part1(m2);
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

    int r2 = part2(mapa);
    std::cout << "2: " << r2 << "\n";

    return 0;
}
