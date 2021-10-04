#include <boost/container_hash/hash.hpp>
#include <cassert>
#include <iostream>
#include <regex>

struct XYZ {
    long x = 0, y = 0, z = 0;

    XYZ() = default;
    XYZ(long x_, long y_, long z_) noexcept
        : x {x_}
        , y {y_}
        , z {z_}
    { }

    long sum() const noexcept { return std::abs(x) + std::abs(y) + std::abs(z); }

    bool operator==(XYZ const& o) const noexcept { return x == o.x && y == o.y && z == o.z; }

    XYZ& operator+= (XYZ const& o) noexcept {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
};

struct Asteroid {
    XYZ position;
    XYZ velocity;

    bool operator== (Asteroid const& o) const noexcept {
        return position == o.position;
    }

    void move() {
        position += velocity;
    }

    void update_velocity(Asteroid& o) {
        assert(!(*this == o));

        auto cmp = [](long const& pos1, long const& pos2, long& v1, long& v2) {
            // std::cout << "> cmp(" << pos1 << ", " << pos2 << ")\n";
            if (pos1 < pos2) {
                ++v1;
                --v2;
            } else if (pos1 > pos2) {
                --v1;
                ++v2;
            }
        };

        // std::cout << "X";
        cmp(position.x, o.position.x, velocity.x, o.velocity.x);
        // std::cout << "Y";
        cmp(position.y, o.position.y, velocity.y, o.velocity.y);
        // std::cout << "Z";
        cmp(position.z, o.position.z, velocity.z, o.velocity.z);
    }

    long energy() const noexcept {
        return position.sum() * velocity.sum();
    }
};

namespace std {

template <>
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

std::ostream& operator<<(std::ostream& os, XYZ const& p) {
    os << "(" << p.x << "; " << p.y << "; " << p.z << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, Asteroid const& a) {
    os << "pos=" << a.position << ", vel=" << a.velocity << ", energy: " << a.energy();
    return os;
}

}  // namespace std

using Asteroids = std::vector<Asteroid>;

int main()
{
    static const std::regex LINE_RX{"<x=([-0-9]+), y=([-0-9]+), z=([-0-9]+)>"};

    Asteroids asteroids;

    std::string line;
    while(std::getline(std::cin, line)) {
        std::smatch m;
        if (!std::regex_search(line, m, LINE_RX)) {
            std::cout << "ERROR! Unknown coordinates! [" << line << "]\n";
            exit(1);
        }
        Asteroid a;
        a.position = XYZ{std::stol(m.str(1)), std::stol(m.str(2)), std::stol(m.str(3))};
        asteroids.push_back(std::move(a));
    }

    for (unsigned i = 0; i < 1000; ++i) {
        for (unsigned a = 0; a < asteroids.size() - 1; ++a) {
            for (unsigned b = a + 1; b < asteroids.size(); ++b) {
                asteroids[a].update_velocity(asteroids[b]);
            }
        }
        for (auto& a : asteroids) {
            a.move();
        }
    }

    long e = 0;
    for (auto const& a : asteroids) {
        e = e + a.energy();
    }
    std::cout << "System energy: " << e << "\n";

    return 0;
}
