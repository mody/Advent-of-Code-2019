#include <array>
#include <boost/container_hash/hash.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>

enum
{
    TEST_X = 0x1,
    TEST_Y = 0x2,
    TEST_Z = 0x4
};

struct XYZ
{
    long x = 0, y = 0, z = 0;

    XYZ() = default;
    XYZ(long x_, long y_, long z_)
    noexcept
        : x {x_}
        , y {y_}
        , z {z_}
    { }

    long sum() const noexcept { return std::abs(x) + std::abs(y) + std::abs(z); }

    bool operator==(XYZ const& o) const noexcept { return (x == o.x) && (y == o.y) && (z == o.z); }

    XYZ& operator+=(XYZ const& o) noexcept
    {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
};

struct Asteroid
{
    bool operator==(Asteroid const& o) const noexcept { return position == o.position && velocity == o.velocity; }

    unsigned eq(Asteroid const& o) {
        unsigned result = 0;
        if (position.x == o.position.x && velocity.x == o.velocity.x) { result |= TEST_X; }
        if (position.y == o.position.y && velocity.y == o.velocity.y) { result |= TEST_Y; }
        if (position.z == o.position.z && velocity.z == o.velocity.z) { result |= TEST_Z; }
        return result;
    }

    void move() { position += velocity; }

    void update_velocity(Asteroid& o)
    {
        auto cmp = [](long const& pos1, long const& pos2, long& v1, long& v2) {
            if (pos1 == pos2) return;

            if (pos1 < pos2) {
                ++v1;
                --v2;
            } else {
                --v1;
                ++v2;
            }
        };

        cmp(position.x, o.position.x, velocity.x, o.velocity.x);
        cmp(position.y, o.position.y, velocity.y, o.velocity.y);
        cmp(position.z, o.position.z, velocity.z, o.velocity.z);
    }

    long energy() const noexcept { return position.sum() * velocity.sum(); }

    XYZ position;
    XYZ velocity;
};

using Asteroids = std::array<Asteroid, 4>;

namespace std {

template<>
struct hash<XYZ>
{
    size_t operator()(XYZ const& p) const noexcept
    {
        size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        boost::hash_combine(seed, p.z);
        return seed;
    }
};

template<>
struct hash<Asteroid>
{
    size_t operator()(Asteroid const& a) const noexcept
    {
        size_t seed = 0;
        boost::hash_combine(seed, std::hash<XYZ> {}(a.position));
        boost::hash_combine(seed, std::hash<XYZ> {}(a.velocity));
        return seed;
    }
};

std::ostream& operator<<(std::ostream& os, XYZ const& p)
{
    os << "(" << p.x << "; " << p.y << "; " << p.z << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, Asteroid const& a)
{
    os << "pos=" << a.position << ", vel=" << a.velocity << ", energy: " << a.energy();
    return os;
}

}  // namespace std

void update_velocity(Asteroids& asteroids)
{
    asteroids[0].update_velocity(asteroids[1]);
    asteroids[0].update_velocity(asteroids[2]);
    asteroids[0].update_velocity(asteroids[3]);
    asteroids[1].update_velocity(asteroids[2]);
    asteroids[1].update_velocity(asteroids[3]);
    asteroids[2].update_velocity(asteroids[3]);
}

long part1(Asteroids asteroids)
{
    for (unsigned i = 0; i < 1000; ++i) {
        update_velocity(asteroids);
        for (auto& a : asteroids) {
            a.move();
        }
    }

    long e = 0;
    for (auto const& a : asteroids) {
        e = e + a.energy();
    }
    return e;
}

long gcd(long a, long b)
{
    if (a == 0) { return b; }
    return gcd(b % a, a);
}

long lcm(long a, long b)
{
    return std::abs(a * b) / gcd(a, b);
}

long gcd(std::vector<long> arr)
{
    if (arr.empty()) {
        return 0;
    } else if (arr.size() == 1) {
        return arr.at(0);
    }

    long g = gcd(arr.at(0), arr.at(1));
    for (size_t i = 2; i < arr.size(); ++i) {
        g = gcd(g, arr.at(i));
    }
    return g;
}

long lcm(std::vector<long> arr)
{
    if (arr.empty()) {
        return 0;
    } else if (arr.size() == 1) {
        return arr.at(0);
    }

    long g = lcm(arr.at(0), arr.at(1));
    for (size_t i = 2; i < arr.size(); ++i) {
        g = lcm(g, arr.at(i));
    }

    return g;
}

unsigned long part2(Asteroids asteroids)
{
    const Asteroids orig = asteroids;

    std::map<unsigned, long> steps;

    for (unsigned long i = 1, j = 0; j < 3; ++i) {
        update_velocity(asteroids);

        for (auto& a : asteroids) {
            a.move();
        }

        const unsigned eq = asteroids[0].eq(orig[0]);
        if ((eq == TEST_X || eq == TEST_Y || eq == TEST_Z) && eq == asteroids[1].eq(orig[1])
            && eq == asteroids[2].eq(orig[2]) && eq == asteroids[3].eq(orig[3]))
        {
            if (steps.insert({eq, i}).second) {
                ++j;
            }
        }
    }

    return lcm({steps.at(1), steps.at(2), steps.at(4)});
}

int main()
{
    static const std::regex LINE_RX {"<x=([-0-9]+), y=([-0-9]+), z=([-0-9]+)>"};

    Asteroids asteroids;

    std::string line;
    for (unsigned i = 0; i < 4 && std::getline(std::cin, line); ++i) {
        std::smatch m;
        if (!std::regex_search(line, m, LINE_RX)) {
            std::cout << "ERROR! Unknown coordinates! [" << line << "]\n";
            exit(1);
        }
        Asteroid a;
        a.position = XYZ {std::stol(m.str(1)), std::stol(m.str(2)), std::stol(m.str(3))};
        asteroids[i] = std::move(a);
    }

    std::cout << "System energy: " << part1(asteroids) << "\n";
    std::cout << "Duplicity on: " << part2(asteroids) << "\n";


    return 0;
}
