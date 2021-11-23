#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <vector>

size_t to_int(char c)
{
    assert(c >= '0');
    assert(c <= '9');
    return c - '0';
}

char to_char(size_t num)
{
    assert(num < 10);
    return '0' + num;
}

struct PatternIterator
{
    enum OP
    {
        MINUS = -1,
        SKIP = 0,
        PLUS = 1,
    };

    // static const Pattern base_pattern = {0, 1, 0, -1};
    PatternIterator(size_t _index, size_t _size)
        : repeat {_index + 1}
        , size {_size}
        , pattern_size {4 * repeat}
        , input_pos {_index}
        , pattern_pos {(1 + _index) % pattern_size}
    {
        assert(get().second == PLUS);
    }


    bool end() const noexcept
    {
        return input_pos >= size;
    }

    std::pair<size_t, OP> get() const noexcept
    {
        switch ((pattern_pos / repeat) % 4) {
        case 0:  // skip1
        case 2:  // skip2
            return {input_pos, SKIP};
        case 1:  // plus
            return {input_pos, PLUS};
        case 3:  // minus
            return {input_pos, MINUS};
        };
        assert(false);
    }

    bool next() noexcept
    {
        assert(get().second != SKIP);
        ++input_pos;
        ++pattern_pos;
        if (pattern_pos == pattern_size) {
            pattern_pos = 0;
        }
        if (get().second == SKIP) {
            input_pos += repeat;
            pattern_pos += repeat;
            if (pattern_pos >= pattern_size) {
                pattern_pos = pattern_pos % pattern_size;
            }
        }
        return !end();
    }

protected:
    const size_t repeat;
    const size_t size;
    const size_t pattern_size;

    size_t input_pos;
    size_t pattern_pos;
};

std::string process(std::string signal, size_t offset = 0)
{
    const auto sig_size = signal.size();

    for (auto& c : signal) {
        c = to_int(c);
    }

    for (unsigned iteration = 0; iteration < 100; ++iteration) {
        std::string new_signal(signal.begin(), signal.end());

        for (size_t index = 0; index < sig_size; ++index) {
            PatternIterator pi(index, sig_size);
            long long num = 0;
            do {
                auto it = pi.get();
                num += signal.at(it.first) * it.second;
            } while (pi.next());

            char& c = new_signal[index];
            c = (std::abs(num) % 10);
        }

        std::swap(signal, new_signal);
    }

    for (auto& c : signal) {
        c = to_char(c);
    }
    return signal;
}

std::string process2(std::string signal, size_t offset)
{
    const auto sig_size = signal.size();

    for (auto& c : signal) {
        c = to_int(c);
    }

    for (unsigned iteration = 0; iteration < 100; ++iteration) {
        std::string new_signal(signal.begin(), signal.end());

        size_t num = 0;
        for (size_t index = sig_size - 1; index >= offset; --index) {
            num += signal.at(index);
            char& c = new_signal[index];
            c = num % 10;
        }

        std::swap(signal, new_signal);
    }

    for (auto& c : signal) {
        c = to_char(c);
    }
    return signal;
}


void part_1(std::string signal)
{
    signal = process(signal);
    std::cout << "result_1:\t" << signal.substr(0, 8) << "\n";
}

void part_2(std::string signal)
{
    const unsigned long offset = std::stol(signal.substr(0, 7));

    constexpr int MULTIPLY = 10000;

    std::string long_signal;
    long_signal.reserve(MULTIPLY * signal.size());

    for (unsigned i = 0; i < MULTIPLY; ++i) {
        std::copy(signal.begin(), signal.end(), std::back_inserter(long_signal));
    }

    long_signal = process2(long_signal, offset);
    std::cout << "result_2:\t" << long_signal.substr(offset, 8) << "\n";
}

int main()
{
    std::string signal;
    if (!std::getline(std::cin, signal)) {
        std::cerr << "Missing input\n";
        return 1;
    }

    part_1(signal);
    part_2(signal);

    return 0;
}
