#include <algorithm>
#include <boost/config.hpp>
#include <boost/container_hash/hash.hpp>

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>


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

    constexpr Point(value_t x_, value_t y_) noexcept : Point(x_, y_, 0) {}

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


using Mapa = std::unordered_map<Point, unsigned char>;


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
    // return {0, 0, 0, 4, 4, 0};
}


std::string to_string(Mapa const& mapa, Point::value_t z = 0)
{
    std::stringstream ss;

    auto const& [min_x, min_y, min_z, max_x, max_y, max_z] = mapa_min_max(mapa);
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x) {
            auto it = mapa.find({x, y, z});
            if (it == mapa.end()) {
                ss << "?";
                continue;
            }
            ss << it->second;
        }
        ss << "\n";
    }
    return ss.str();
}


namespace std {

template<>
struct hash<Mapa>
{
    size_t operator()(Mapa const& m) const noexcept
    {
        return std::hash<std::string>()(to_string(m));
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



void dump(Mapa const& mapa, Point::value_t z = 0)
{
    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << to_string(mapa, z);
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
            mapa.insert({p0, c});
            ++p0.x;
        }
        p0.x = 0;
        ++p0.y;
    }
    return mapa;
}


void part1(Mapa mapa)
{
    std::unordered_set<Mapa> seen;

    seen.insert(mapa);

    for (;;) {
        Mapa m2;
        for (auto const& [px, c] : mapa) {
            assert(px.z == 0);
            const auto north = px + Direction::NORTH;
            const auto south = px + Direction::SOUTH;
            const auto west = px + Direction::WEST;
            const auto east = px + Direction::EAST;

            unsigned around = 0;
            if (mapa.contains(north) && mapa.at(north) == '#') {
                ++around;
            }
            if (mapa.contains(south) && mapa.at(south) == '#') {
                ++around;
            }
            if (mapa.contains(west) && mapa.at(west) == '#') {
                ++around;
            }
            if (mapa.contains(east) && mapa.at(east) == '#') {
                ++around;
            }

            if (c == '.' && (around == 1 || around == 2)) {
                m2[px] = '#';
            } else if (c == '#' && around != 1) {
                m2[px] = '.';
            } else {
                m2[px] = c;
            }
        }

        std::swap(mapa, m2);

        if (!seen.insert(mapa).second)
        {
            break;
        }
    }

    int64_t result = 0, increment = 1;

    auto const& [min_x, min_y, min_z, max_x, max_y, max_z] = mapa_min_max(mapa);
    for (Point::value_t y = min_y; y <= max_y; ++y) {
        for (Point::value_t x = min_x; x <= max_x; ++x, increment <<= 1) {
            auto it = mapa.find({x, y, 0});
            if (it == mapa.end()) {
                continue;
            }
            if (it->second == '#') {
                result += increment;
            }
        }
    }

    std::cout << "1: " << result << std::endl;
    // -------------------------------------------------------------------------------
    // ....#
    // ....#
    // ##..#
    // ##..#
    // .#..#
    // -------------------------------------------------------------------------------
    // 1: 19516944
}


struct Level;
using LevelPtr = std::unique_ptr<Level>;


struct Level
{
    Level() noexcept {
        for (int64_t y = 0; y <= MAX_Y; ++y) {
            for (int64_t x = 0; x <= MAX_X; ++x) {
                if (x == INNER_X && y == INNER_Y) {
                    continue;
                }
                m1.insert({{x, y}, '.'});
            }
        }
    }

    Level(Mapa _m) noexcept
        : m1 {std::move(_m)}
    {
        m1.erase({2, 2}); // remove the inner point
    }

    unsigned char get(Point const& px) const { return m1.at(px); }

    unsigned int count_all_north() const noexcept
    {
        return count_all({0, 0}, Direction::EAST);
    }

    unsigned int count_all_south() const noexcept
    {
        return count_all({MAX_X, MAX_Y}, Direction::WEST);
    }

    unsigned int count_all_west() const noexcept
    {
        return count_all({0, 0}, Direction::SOUTH);
    }

    unsigned int count_all_east() const noexcept
    {
        return count_all({MAX_X, MAX_Y}, Direction::NORTH);
    }

    bool empty() const noexcept {
        return std::all_of(m1.begin(), m1.end(), [](auto const& i) { return i.second == '.'; });
    }

    void swap_maps() noexcept {
        std::swap(m1, m2);
        m2 = {};
    }

    unsigned bug_count() const noexcept {
        unsigned cnt {0};
        for (auto const& [_, c] : m1) {
            if (c == '#') {
                ++cnt;
            }
        }
        return cnt;
    }

    void step() noexcept
    {
        static constexpr Point outer_north{2, 1};
        static constexpr Point outer_south{2, 3};
        static constexpr Point outer_west{1, 2};
        static constexpr Point outer_east{3, 2};

        for (auto const& [px, c] : m1) {
            assert(px.z == 0);
            assert(!(px == Point{INNER_X, INNER_Y, 0}));

            unsigned around = 0;

            // std::cout << std::hex << this << std::dec << "; Examine " << px << ", c: " << c << "; ";

            const Point north = px + Direction::NORTH;
            const Point south = px + Direction::SOUTH;
            const Point west = px + Direction::WEST;
            const Point east = px + Direction::EAST;


            if (north.y < MIN_Y) {
                if (outer && outer->get(outer_north) == '#') {
                    // std::cout << "check outer_north; ";
                    ++around;
                }
            } else if (north.x == INNER_X && north.y == INNER_Y) {
                if (inner) {
                    // std::cout << "check all_south; ";
                    around += inner->count_all_south();
                }
            } else if (m1.contains(north) && m1.at(north) == '#') {
                // std::cout << "check one_north; ";
                ++around;
            }


            if (south.y > MAX_Y) {
                if (outer && outer->get(outer_south) == '#') {
                    // std::cout << "check outer_south; ";
                    ++around;
                }
            } else if (south.x == INNER_X && south.y == INNER_Y) {
                if (inner) {
                    // std::cout << "check all_north; ";
                    around += inner->count_all_north();
                }
            } if (m1.contains(south) && m1.at(south) == '#') {
                // std::cout << "check one_south; ";
                ++around;
            }


            if (west.x < MIN_X) {
                if (outer && outer->get(outer_west) == '#') {
                    // std::cout << "check outer_west; ";
                    ++around;
                }
            } else if (west.x == INNER_X && west.y == INNER_Y) {
                if (inner) {
                    // std::cout << "check all_west; ";
                    around += inner->count_all_east();
                }
            } else if (m1.contains(west) && m1.at(west) == '#') {
                // std::cout << "check one_west; ";
                ++around;
            }


            if (east.x > MAX_X) {
                if (outer && outer->get(outer_east) == '#') {
                    // std::cout << "check outer_east; ";
                    ++around;
                }
            } else if (east.x == INNER_X && east.y == INNER_Y) {
                if (inner) {
                    // std::cout << "check all_east; ";
                    around += inner->count_all_west();
                }
            } else if (m1.contains(east) && m1.at(east) == '#') {
                // std::cout << "check one_east; ";
                ++around;
            }

            // std::cout << "; around=" << around << "; ";

            if (c == '.' && (around == 1 || around == 2)) {
                m2[px] = '#';
                // std::cout << "born";
            } else if (c == '#' && around != 1) {
                m2[px] = '.';
                // std::cout << "died";
            } else {
                m2[px] = c;
                assert(m2.at(px) == c);
                // std::cout << "noop";
            }
            // std::cout << std::endl;
        }
    }

    void add_outer(Level* l) noexcept
    {
        assert(!outer);
        outer = l;
        l->inner = this;
    }

    void add_inner(Level* l) noexcept
    {
        assert(!inner);
        inner = l;
        l->outer = this;
    }

    static Level* find_top(Level* me) noexcept
    {
        while(me->outer) {
            me = me->outer;
        }
        return me;
    }

    static Level* find_bottom(Level* me) noexcept
    {
        while(me->inner) {
            me = me->inner;
        }
        return me;
    }

    static void top_to_bottom(Level* me, std::function<void(Level*)>&& op) noexcept
    {
        assert(me);
        do {
            op(me);
            me = me->inner;
        } while (me);
    }

    void dump() const noexcept
    {
        std::cout << "--------------------------------------------------------------------\n";
        std::cout << std::hex << "outer: 0x" << outer << "\n";
        std::cout << std::hex << "me: " << this << "\n";
        std::cout << to_string(m1) << "\n";
        std::cout << std::hex << "inner: 0x" << inner << "\n";
        std::cout << "--------------------------------------------------------------------" << std::endl;
    }

private:

    unsigned count_all(Point px, Direction d) const noexcept
    {
        unsigned cnt {0};

        for (; px.x >= 0 && px.x <= MAX_X && px.y >= 0 && px.y <= MAX_Y; px = px + d) {
            if (m1.at(px) == '#') {
                ++cnt;
            }
        }
        return cnt;
    }

    Level* outer {};
    Level* inner {};
    Mapa m1, m2;

    static constexpr int64_t MIN_X = 0, MAX_X = 4, INNER_X = 2;
    static constexpr int64_t MIN_Y = 0, MAX_Y = 4, INNER_Y = 2;
};


struct Levels
{
    Level* add_level() noexcept {
        levels.push_back(std::make_unique<Level>());
        return levels.back().get();
    }

    Level* add_level(Mapa _m) noexcept {
        levels.push_back(std::make_unique<Level>(std::move(_m)));
        return levels.back().get();
    }

private:
    std::vector<LevelPtr> levels;
};


void part2(Mapa const& mapa)
{
    Levels levels;

    Level *top = levels.add_level(std::move(mapa)), *bottom = top;

    for (unsigned i = 0; i < 200; ++i) {

        // top = Level::find_top(top);
        // bottom = Level::find_bottom(top);

        if (!top->empty()) {
            Level* l = levels.add_level();
            top->add_outer(l);
            top = l;
        }

        if (!bottom->empty()) {
            Level* l = levels.add_level();
            bottom->add_inner(l);
            bottom = l;
        }

        // top = Level::find_top(top);
        // bottom = Level::find_bottom(top);

        // std::cout << "BEFORE\n";
        // Level::top_to_bottom(top, [](LevelPtr me) {
        //     assert(me);
        //     me->dump();
        // });


        Level::top_to_bottom(top, [](Level* me) {
            assert(me);
            me->step();
        });

        Level::top_to_bottom(top, [](Level* me) {
            assert(me);
            me->swap_maps();
        });

        unsigned bug_count {};
        Level::top_to_bottom(top, [&bug_count](Level* me) {
            assert(me);
            bug_count += me->bug_count();
        });

        // std::cout << "AFTER\n";
        // Level::top_to_bottom(top, [](Level* me) {
        //     assert(me);
        //     me->dump();
        // });

        // std::cout << std::dec << "bugs @ " << i << ": " << bug_count << std::endl;
    }


    // top = Level::find_top(top);
    // auto bottom = Level::find_bottom(top);

    // Level::top_to_bottom(top, [](Level* me) {
    //     assert(me);
    //     me->dump();
    // });

    unsigned bug_count {};
    Level::top_to_bottom(top, [&bug_count](Level* me) {
        assert(me);
        bug_count += me->bug_count();
    });

    std::cout << "2: " << std::dec << bug_count << std::endl;
}


int main(int argc, char* argv[])
{
    Mapa mapa = load_map();

    part1(mapa);
    part2(mapa);

    return 0;
}

