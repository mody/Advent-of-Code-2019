#include <cassert>
#include <iostream>
#include <iterator>
#include <vector>

using Pattern = std::vector<char>;
using Patterns = std::vector<Pattern>;

Pattern base_pattern = {0, 1, 0, -1};

Pattern multiply(unsigned times)
{
    assert(times > 0);
    if (times == 1) {
        return base_pattern;
    }
    Pattern result;
    for (auto x : base_pattern) {
        for (unsigned i = 0; i < times; ++i) {
            result.push_back(x);
        }
    }
    return result;
}

Patterns initialize(unsigned size)
{
    Patterns pats;
    pats.push_back({});  // index 0 is empty

    for (unsigned i = 1; i <= size; ++i) {
        Pattern p = multiply(i);
        while (p.size() < size) {
            Pattern pp = p;
            std::copy(p.begin(), p.end(), std::back_inserter(pp));
            std::swap(pp, p);
        }
        p.erase(p.begin());
        p.resize(size);
        pats.push_back(std::move(p));
    }
    return pats;
}

unsigned to_int(char c)
{
    assert(c >= '0');
    assert(c <= '9');
    return c - '0';
}

char to_char(int num)
{
    assert(num < 10);
    return '0' + num;
}

int main()
{
    {
        assert(multiply(1) == base_pattern);
        Pattern t {0, 0, 1, 1, 0, 0, -1, -1};
        assert(multiply(2) == t);
    }

    std::string signal;
    if (!std::getline(std::cin, signal)) {
        std::cerr << "Missing input\n";
        return 1;
    }
    const auto sig_size = signal.size();

    std::cout << "signal: " << signal << "\n";
    std::cout << "length: " << sig_size << "\n";

    Patterns pats = initialize(sig_size);
    // for (unsigned i = 0; i < pats.size(); ++i) {
    //     std::cout << i << ": {";
    //     for (auto const& x : pats.at(i)) {
    //         std::cout << (int)x << ", ";
    //     }
    //     std::cout << "}\n";
    // }

    for(unsigned iteration = 0; iteration < 100; ++iteration) {
        // std::cout << "iteration: " << iteration << "\t" << signal << "\n";
        auto new_signal = signal;

        unsigned index = 1;
        for (auto& c : new_signal) {
            int num = 0;
            Pattern const& p = pats[index];
            for (unsigned i = 0; i < sig_size; ++i) {
                // std::cout << to_int(signal.at(i)) << "*" << (int)p.at(i) << " + ";
                num += to_int(signal.at(i)) * p.at(i);
            }
            num = std::abs(num);
            c = to_char(num % 10);
            // std::cout << " = " << num << " -> " << (num %10) << "\n";
            ++index;
        }
        std::swap(signal, new_signal);
    }
    std::cout << "last:\t" << signal.substr(0, 8) << "\n";

    return 0;
}
