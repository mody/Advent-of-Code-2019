#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>
#include <vector>

#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>

#include <chrono>

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

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
    enum OP {
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


    bool end() const noexcept {
        // std::cout << "\tAtEnd? " << (input_pos >= size) << "\n";
        return input_pos >= size;
    }

    std::pair<size_t, OP> get() const noexcept {
        switch ((pattern_pos / repeat) % 4) {
        case 0: // skip1
        case 2: // skip2
            // std::cout << "\tpattern_pos: " << pattern_pos << " is SKIP\n";
            return {input_pos, SKIP};
        case 1: // plus
            // std::cout << "\tpattern_pos: " << pattern_pos << " is PLUS\n";
            return {input_pos, PLUS};
        case 3: // minus
            // std::cout << "\tpattern_pos: " << pattern_pos << " is MINUS\n";
            return {input_pos, MINUS};
        };
        assert(false);
    }

    bool next() noexcept {
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
    std::cout << "Processing signal of length: " << sig_size << " from offset: " << offset << "\n";

    for (auto& c : signal) {
        c = to_int(c);
    }

    for (unsigned iteration = 0; iteration < 100; ++iteration) {
        std::cout << "iteration: " << iteration;
        const auto t1 = high_resolution_clock::now();

        tbb::concurrent_vector<char> new_signal(signal.begin(), signal.end());

        auto processor = [&](const tbb::blocked_range<size_t>& r) {
            for (size_t index = r.begin(); index != r.end(); ++index) {
                // std::cout << "Index: " << index << "; signal: " << signal << "\n";
                PatternIterator pi(index, sig_size);
                long long num = 0;
                do {
                    auto it = pi.get();
                    // std::cout << "\t(" << it.first << "; " << it.second << "): " << to_int(signal.at(it.first)) << " * "
                    //           << it.second << std::endl;
                    num += signal.at(it.first) * it.second;
                } while (pi.next());

                char& c = new_signal[index];
                c = (std::abs(num) % 10);
                // std::cout << "num: " << num << "; c: " << c << "\n";
            }
        };

        tbb::parallel_for(tbb::blocked_range<size_t>(offset, sig_size), processor);
        // processor(tbb::blocked_range<size_t>(offset, sig_size));

        signal = std::string(new_signal.begin(), new_signal.end());

        const auto t2 = high_resolution_clock::now();
        const auto duration = duration_cast<milliseconds>(t2 - t1);
        std::cout << " took " << duration.count() << "ms\n";
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
    std::string long_signal;

    constexpr int MULTIPLY = 10000;

    long_signal.reserve(MULTIPLY*signal.size());

    for (unsigned i=0; i < MULTIPLY; ++i) {
        std::copy(signal.begin(), signal.end(), std::back_inserter(long_signal));
    }

    long_signal = process(long_signal, offset);
    std::cout << "result_2:\t" << long_signal.substr(offset, 8) << "\n";
}

int main()
{
    std::string signal;
    if (!std::getline(std::cin, signal)) {
        std::cerr << "Missing input\n";
        return 1;
    }

    std::cout << "signal: " << signal << "\n";

    part_1(signal);
    part_2(signal);

    return 0;
}
