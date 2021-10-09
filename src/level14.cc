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

std::ostream& operator<<(std::ostream& os, Item const& i) {
    std::cout << "(" << i.name << ", " << i.amount << ")";
    return os;
}

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
        // std::cout << "Compress " << queue << "\n";

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
        // std::cout << "Compressed " << queue << "\n";
        return edit;
    }

    void optimize(Items& queue) const
    {
        bool edited = false;

        do {
            // std::cout << "\nSolving " << queue << "\n";

            Items q2;

            for (auto qit = queue.begin(); qit != queue.end(); ++qit) {
                auto const& rq = *qit;

                auto it = recipe.find(rq);
                if (it == recipe.end()) {
                    q2.push_back(rq);
                    continue;  // can't expand more
                }

                // std::cout << "Can replace " << rq << "?";

                if (rq.amount < it->first.amount || rq.amount % it->first.amount) {
                    q2.push_back(rq);
                    // std::cout << " - no\n";
                    continue;
                }

                unsigned mul = rq.amount / it->first.amount;
                // std::cout << " - yes x" << mul << "\n";

                for (Item item : it->second) {
                    item.amount *= mul;
                    q2.push_back(std::move(item));
                }
            }

            // std::cout << "Solved " << q2 << "\n";

            edited = (q2 != queue);
            queue = std::move(q2);

        } while (edited);
    }

    void make(Item rq)
    {
        // std::cout << "Make " << rq << "\n";
        if (rq.name == "ORE") {
            ore += rq.amount;
            return;
        }

        int64_t& stored = storage[rq.name];

        if (stored >= rq.amount) {
            stored -= rq.amount;
            return;
        }

        auto rqr = recipe.find(rq);
        assert(rqr != recipe.end());

        for (;;) {
            // std::cout << "still need " << rq << "\n";

            for (auto const& need : rqr->second) {
                make(need);
            }

            stored += rqr->first.amount;
            // std::cout << "Made " << rqr->first << " have " << *rqs << "\n";

            if (stored >= rq.amount) {
                stored -= rq.amount;
                // std::cout << "Made " << rq << " from storage, left " << *rqs << "\n";
                break;
            }

        }
    }

    void clear()
    {
        storage.clear();
        ore = 0;
    }

    const Recipe recipe;
    std::unordered_map<std::string, int64_t> storage;
    int64_t ore = 0;
};

int64_t part1(Recipe const& rec)
{
    Items queue;
    queue.push_front({"FUEL", 1});

    bool edited = false;

    Lab lab(rec);

    do {
        lab.optimize(queue);
        edited = lab.compress(queue);
    } while (edited);

    for(auto const& i : queue) {
        lab.make(i);
    }

    // std::cout << "Done\n";

    return lab.ore;
}


int64_t part2(Recipe const& rec)
{
    int64_t one = 0;
    {
        Lab lab(rec);

        Items queue;
        queue.push_front({"FUEL", 1});

        std::cout << queue.front();

        bool edited = false;
        do {
            lab.optimize(queue);
            edited = lab.compress(queue);
        } while (edited);

        for (auto const& i : queue) {
            lab.make(i);
        }

        std::cout << " needs " << lab.ore << std::endl;
        one = lab.ore;
    }

    Lab lab(rec);

    Items queue;
    queue.push_front({"FUEL", 1000000000000 / one});

    std::cout << queue.front();

    bool edited = false;
    do {
        lab.optimize(queue);
        edited = lab.compress(queue);
    } while (edited);

    for (auto const& i : queue) {
        lab.make(i);
    }

    std::cout << " needs " << lab.ore << std::endl;

    return (1000000000000./one)*(1000000000000. / lab.ore);
    // print(int(int(1000000000000 / one_fuel)*(1000000000000 / calculate_ore(int(1000000000000 / one_fuel)))))

    return 0;
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
