#include <algorithm>
#include <array>
#include <bits/iterator_concepts.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>


template<int64_t SIZE, int64_t TARGET>
struct Calc
{
    void apply_reverse() noexcept {
        position = SIZE - position - 1;
    }

    void apply_cut(int arg) noexcept {
        position = (position + (SIZE - arg)) % SIZE;
    }

    void apply_step(int arg) noexcept {
        position = (position * arg) % SIZE;
    }

    std::string to_string() const noexcept {
        std::stringstream ss;
        ss << "(target: " << TARGET << "; pos=" << position << "; " << SIZE << ")";
        return ss.str();
    }

    bool operator< (const Calc& o) const noexcept {
        return position < o.position;
    }

private:
    int64_t position {TARGET};
};


enum class Op
{
    REVERSE,
    CUT,
    STEP,
};

struct Instruction
{
    Op op {};
    int arg {};

};

using Instructions = std::vector<Instruction>;


namespace std {

std::ostream& operator<<(std::ostream& os, Instruction const& i)
{
    if (i.op == Op::CUT) {
        os << "CUT " << i.arg;
    } else if (i.op == Op::STEP) {
        os << "STEP " << i.arg;
    } else if (i.op == Op::REVERSE) {
        os << "REVERSE";
    } else {
        assert(false);
    }
    return os;
}

}  // namespace std


void run1(const Instructions& orders)
{
    Calc<10007, 2019> calc {};

    for (auto const& order : orders) {
        // std::cout << "ORDER: " << order << std::endl;
        if (order.op == Op::CUT) {
            calc.apply_cut(order.arg);
        } else if (order.op == Op::STEP) {
            calc.apply_step(order.arg);
        } else if (order.op == Op::REVERSE) {
            calc.apply_reverse();
        } else {
            assert(false);
        }
    }

    std::cout << "1: " << calc.to_string() << std::endl;
}


void run2(const Instructions& orders)
{
    using Value = Calc<119315717514047, 2020>;

    Value calc {};

    // std::set<Value> visited;

    for (uint64_t i = 0; i < 101741582076661; ++i) {
        for (auto const& order : orders) {
            // std::cout << "ORDER: " << order << std::endl;
            if (order.op == Op::CUT) {
                calc.apply_cut(order.arg);
            } else if (order.op == Op::STEP) {
                calc.apply_step(order.arg);
            } else if (order.op == Op::REVERSE) {
                calc.apply_reverse();
            } else {
                assert(false);
            }
        }

        // if (auto const& [_, inserted] = visited.insert(calc); !inserted) {
        //     std::cout << "Cycle at iteration " << i << std::endl;
        //     break;
        // }

        if ((i % 10000000) == 0) {
            std::cout << std::fixed << i << ", " << ((100.0*i)/119315717514047) << "%" << std::endl;
        }
    }

    std::cout << "2: " << calc.to_string() << std::endl;
}


int main()
{
    Instructions orders;

    std::string line;
    while(std::getline(std::cin, line))
    {
        if (line.empty()) break;

        Instruction i {};

        if (line.starts_with("cut ")) {
            i.op = Op::CUT;
            i.arg = std::stoi(&line[4]);
        } else if (line.starts_with("deal with ")) {
            i.op = Op::STEP;
            i.arg = std::stoi(&line[20]);
        } else if (line.starts_with("deal into ")) {
            i.op = Op::REVERSE;
        } else {
            assert(false);
        }

        orders.push_back(std::move(i));
    }

    run1(orders);
    run2(orders);

    return 0;
}
