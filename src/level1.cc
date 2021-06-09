#include <iostream>
#include <string>
#include <vector>

int calcFuel(unsigned mass) {
    return mass / 3 - 2;
}


int main(int argc, char* argv[]) {
    unsigned sum1 = 0, sum2 = 0;

    for (;;) {
        unsigned mass = 0;
        std::cin >> mass;
        if (std::cin.eof()) { break; }
        int fuel = calcFuel(mass);
        sum1 += fuel;
        while(fuel > 0) {
            sum2 += fuel;
            fuel = calcFuel(fuel);
        }
    }

    std::cout << "1: " << sum1 << std::endl << "2: " << sum2 << std::endl;
    return 0;
}
