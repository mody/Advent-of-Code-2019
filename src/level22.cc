#include <algorithm>
#include <array>
#include <bits/iterator_concepts.h>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

// constexpr unsigned int CARDS = 10;
constexpr unsigned int CARDS = 10007;
using Deck = std::array<unsigned int, CARDS>;

enum class Op
{
    REVERSE,
    CUT,
    OFFSET,
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
    } else if (i.op == Op::OFFSET) {
        os << "OFFSET " << i.arg;
    } else if (i.op == Op::REVERSE) {
        os << "REVERSE";
    } else {
        assert(false);
    }
    return os;
}

}  // namespace std


Deck apply_offset(Deck const& deck, unsigned int offset)
{
    Deck deck2;
    unsigned int j = 0;
    for (unsigned int i = 0; i < CARDS; ++i) {
        deck2.at(j) = deck.at(i);
        j += offset;
        j = j % deck.size();
    }
    return deck2;
}


Deck apply_cut(Deck deck, int offset)
{
    const unsigned int new_start = offset > 0 ? offset : (deck.size() + offset);
    std::rotate(deck.begin(), deck.begin() + new_start, deck.end());
    return deck;
}


Deck apply_reverse(Deck deck)
{
    std::ranges::reverse(deck);
    return deck;
}


void print_deck(Deck const& deck)
{
    for (auto const& card : deck) {
        std::cout << " " << card;
    }
    std::cout << std::endl;
}


void run1(const Instructions& orders)
{
    Deck deck;

    std::iota(deck.begin(), deck.end(), 0);

    for (auto const& order : orders) {
        // std::cout << "ORDER: " << order << std::endl;
        if (order.op == Op::CUT) {
            deck = apply_cut(deck, order.arg);
        } else if (order.op == Op::OFFSET) {
            deck = apply_offset(deck, order.arg);
        } else if (order.op == Op::REVERSE) {
            deck = apply_reverse(deck);
        } else {
            assert(false);
        }
    }

    // print_deck(deck);
    auto it = std::ranges::find(deck, 2019);
    assert(it != deck.end());
    std::cout << "1: " << std::distance(deck.begin(), it) << std::endl;
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
            i.op = Op::OFFSET;
            i.arg = std::stoi(&line[20]);
        } else if (line.starts_with("deal into ")) {
            i.op = Op::REVERSE;
        } else {
            assert(false);
        }
        orders.push_back(std::move(i));
    }

    run1(orders);

    return 0;
}
