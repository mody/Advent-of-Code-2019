#include <cassert>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

struct Planet {
    std::string name;
    unsigned steps = 0;
};


void compute_steps(
    std::unordered_multimap<std::string, Planet>& data, std::string const& name, unsigned steps)
{
    auto [from, to] = data.equal_range(name);

    for (; from != to; ++from) {
        Planet& p = from->second;
        p.steps = steps;

        compute_steps(data, p.name, steps + 1);
    }
}


int main()
{
    std::unordered_multimap<std::string, Planet> data;
    std::unordered_set<std::string> leafs;

    std::string line;
    while(std::getline(std::cin, line)) {
        auto sep = line.find(')');
        assert(sep != std::string::npos);
        auto center = line.substr(0, sep);
        auto planet = line.substr(sep + 1);

        leafs.erase(center);
        leafs.insert(planet);

        data.insert({center, {planet, 0}});
    }

    compute_steps(data, "COM", 0);

    unsigned total_steps = 0;

    for (auto const& [center, planet] : data) {
        total_steps += planet.steps;
    }

    std::cout << "Connections: " << data.size()+total_steps << "\n";

    return 0;
}
