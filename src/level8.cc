#include <cassert>
#include <iostream>
#include <vector>
#include <map>

using Counts = std::map<unsigned char, unsigned>;

Counts get_counts(std::string const& layer) {
    Counts cnts = {{'0', 0}};
    for (unsigned char c : layer) {
        cnts.insert({c, 0}).first->second++;
    }
    return cnts;
}


int main()
{
    std::string line;
    std::getline(std::cin, line);
    assert(!line.empty());

    constexpr unsigned WIDTH = 25;
    constexpr unsigned HEIGHT = 6;

    assert((line.size() % (WIDTH*HEIGHT)) == 0);

    std::vector<std::string> layers;

    constexpr unsigned LAYER_SIZE = WIDTH*HEIGHT;
    for (unsigned l = 0; l < line.size(); l += LAYER_SIZE) {
        layers.push_back(line.substr(l, LAYER_SIZE));
    }

    Counts best_layer = {{'0', LAYER_SIZE}};

    for (auto const& layer : layers) {
        auto cnts = get_counts(layer);
        if (cnts.at('0') < best_layer.at('0')) {
            best_layer = cnts;
        }
    }

    best_layer.insert({'1', 0}); // some defaults;
    best_layer.insert({'2', 0}); // some defaults;

    std::cout << "1: " << best_layer.at('1')*best_layer.at('2') << "\n";

    return 0;
}
