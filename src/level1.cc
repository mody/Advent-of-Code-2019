#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    unsigned sum = 0;

    for (;;) {
        unsigned num = 0;
        std::cin >> num;
        if (std::cin.eof()) { break; }
        sum += num / 3 - 2;
    }

    std::cout << "1: " << sum << std::endl;
    return 0;
}
