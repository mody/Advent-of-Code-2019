#include <boost/container_hash/hash.hpp>
#include <deque>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct Item {
    std::string name;
    int64_t amount = 0;

    bool operator== (Item const& o) const noexcept {
        return name == o.name;
    }
};

template<>
struct std::hash<Item>
{
    size_t operator()(Item const& r) const noexcept
    {
        size_t seed = 0;
        boost::hash_combine(seed, r.name);
        return seed;
    }
};


using Items = std::deque<Item>;
using Recipe = std::unordered_map<Item, Items>;

std::ostream& operator<<(std::ostream& os, Item const& i) {
    std::cout << "(" << i.name << ", " << i.amount << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, Items const& is) {
    std::cout << "[ ";
    for (Item const& i : is) {
        std::cout << i << ", ";
    }
    std::cout << " ]";
    return os;
}

std::ostream& operator<<(std::ostream& os, Recipe const& rec) {
    for (auto const& [k, v] : rec) {
        std::cout << k << " => " << v << "\n";
    }
    return os;
}

struct Lab {
    Lab(Recipe rec_)
        : recipe {std::move(rec_)}
    { }

    bool compress(Items& queue) const
    {
        bool edit = false;

        std::sort(queue.begin(), queue.end(), [](Item const& a, Item const& b) { return a.name < b.name; });
        for (auto i1 = queue.begin(), i2 = std::next(i1); i2 != queue.end();) {
            if (i1->name == i2->name) {
                i1->amount += i2->amount;
                i2 = queue.erase(i2);
                i1 = queue.begin();
                i2 = std::next(i1);
                edit = true;
            } else {
                ++i1;
                ++i2;
            }
        }
        return edit;
    }

    void optimize(Items& queue) const
    {
        bool edited = false;

        do {
            Items q2;

            for (auto qit = queue.begin(); qit != queue.end(); ++qit) {
                auto const& rq = *qit;

                auto it = recipe.find(rq);
                if (it == recipe.end()) {
                    q2.push_back(rq);
                    continue;  // can't expand more
                }

                if (rq.amount < it->first.amount || rq.amount % it->first.amount) {
                    q2.push_back(rq);
                    continue;
                }

                unsigned mul = rq.amount / it->first.amount;

                for (Item item : it->second) {
                    item.amount *= mul;
                    q2.push_back(std::move(item));
                }
            }

            edited = (q2 != queue);
            queue = std::move(q2);

        } while (edited);
    }

    int64_t make_optimized(Item rq)
    {
        if (rq.name == "ORE") {
            return rq.amount;
        }

        int64_t& stored = storage[rq.name];

        rq.amount -= stored;

        auto rqr = recipe.find(rq);
        assert(rqr != recipe.end());

        const int64_t ratio = (rq.amount + rqr->first.amount - 1) / rqr->first.amount;

        int64_t ore = 0;
        for (auto const& need : rqr->second) {
            ore += make_optimized({need.name, need.amount * ratio});
        }

        stored = rqr->first.amount * ratio - rq.amount;

        return ore;
    }

    const Recipe recipe;
    std::unordered_map<std::string, int64_t> storage;
};

int64_t part1(Recipe const& rec)
{
    Items queue;
    queue.push_front({"FUEL", 1});

    Lab lab(rec);

    bool edited = false;
    do {
        lab.optimize(queue);
        edited = lab.compress(queue);
    } while (edited);

    int64_t ore = 0;
    for(auto const& i : queue) {
        ore += lab.make_optimized(i);
    }

    return ore;
}


int64_t part2(Recipe const& rec)
{
    auto computeOre = [&rec](int64_t value) -> int64_t
    {
        Lab lab(rec);

        Items queue;
        queue.push_front({"FUEL", value});

        bool edited = false;
        do {
            lab.optimize(queue);
            edited = lab.compress(queue);
        } while (edited);

        int64_t ore = 0;
        for (auto const& i : queue) {
            ore += lab.make_optimized(i);
        }
        return ore;
    };

    int64_t first = 1, last = 100000000;
    int64_t it = 0;

    for (int64_t count = (last - first); count > 0;) {
        it = first;
        int64_t step = count / 2;
        it += step;
        if (computeOre(it) < 1000000000000) {
            first = ++it;
            count -= step + 1;
        } else
            count = step;
    }
    return first - 1;
}

int main()
{
    std::cout << "Level14\n";

    Recipe rec;

    static const std::regex LINE_RX {"^([^=]+)=> ([0-9]+) ([A-Z]+)$"};
    static const std::regex ITEMS_RX {"([0-9]+) ([A-Z]+)"};

    std::string line;
    while(std::getline(std::cin, line)) {
        std::smatch m;
        if (!std::regex_search(line, m, LINE_RX)) {
            std::cout << "ERROR! Unknown recipe! [" << line << "]\n";
            exit(1);
        }

        Items& items = rec[Item {m.str(3), static_cast<int64_t>(std::stoul(m.str(2)))}];

        line = m.str(1);
        while (std::regex_search(line, m, ITEMS_RX)) {
            items.push_back(Item{m.str(2), static_cast<int64_t>(std::stoul(m.str(1)))});
            line = m.suffix();
        }
    }

    Items queue;
    queue.push_back(rec.find({"FUEL", 0})->first);

    auto r1 = part1(rec);
    std::cout << "1: " << r1 << "\n";

    auto r2 = part2(rec);
    std::cout << "2: " << r2 << "\n";

    return 0;
}
