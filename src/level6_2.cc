#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <cassert>
#include <fstream>
#include <iostream>

using VertexPropertyType = boost::property<boost::vertex_name_t, std::string>;

using EdgePropertyType = boost::property<boost::edge_weight_t, int>;

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexPropertyType, EdgePropertyType> Graph;
typedef Graph::vertex_descriptor VertexDescriptor;

template<typename T> class TD;

int main() {
    Graph g;

    std::map<std::string, Graph::vertex_descriptor> vx;

    std::string line;
    while (std::getline(std::cin, line)) {
        auto sep = line.find(')');
        assert(sep != std::string::npos);
        auto center = line.substr(0, sep);
        auto planet = line.substr(sep + 1);

        Graph::vertex_descriptor v1, v2;

        if (auto it = vx.find(center); it != vx.end()) {
            v1 = it->second;
        } else {
            v1 = boost::add_vertex({center}, g);
            vx.insert({std::move(center), v1});
        }

        if (auto it = vx.find(planet); it != vx.end()) {
            v2 = it->second;
        } else {
            v2 = boost::add_vertex({planet}, g);
            vx.insert({std::move(planet), v2});
        }

        boost::add_edge(v1, v2, {1}, g);
    }

    // boost::dynamic_properties dp;
    // dp.property("id", boost::get(boost::vertex_name, g));
    // dp.property("weight", boost::get(boost::edge_weight, g));

    // std::ofstream f("g.dot");
    // boost::write_graphviz_dp(f, g, dp, "id");

    // exit(0);

    Graph::vertex_descriptor source = 0, destination = 0;
    auto nameMap = boost::get(boost::vertex_name, g);
    for (auto [first, last] = boost::vertices(g); first != last; ++first) {
        if (nameMap[*first] == "YOU") {
            source = *first;
        } else if (nameMap[*first] == "SAN") {
            destination = *first;
        }
    }

    const int numVertices = num_vertices(g);
    std::vector<unsigned> distances(numVertices);
    std::vector<Graph::vertex_descriptor> pMap(numVertices);

    auto distanceMap =
        predecessor_map(make_iterator_property_map(pMap.begin(), boost::get(boost::vertex_index, g)))
            .distance_map(
                make_iterator_property_map(distances.begin(), boost::get(boost::vertex_index, g)));

    dijkstra_shortest_paths(g, source, distanceMap);

    std::cout << "Dist to target is " << distances[destination] - 2 << "\n";

    return 0;
}
