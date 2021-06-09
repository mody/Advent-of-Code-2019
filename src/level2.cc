#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <iostream>
#include <string>
#include <vector>

using Program = std::vector<unsigned>;

unsigned run_program(Program program)
{
    for (unsigned pc = 0;;)
    {
        if (program.at(pc) == 99) {
            break;
        } else if (program.at(pc) == 1) {
            auto& res = program.at(program.at(pc + 3));
            res = program.at(program.at(pc + 1)) + program.at(program.at(pc + 2));
            pc += 4;
        } else if (program.at(pc) == 2) {
            auto& res = program.at(program.at(pc + 3));
            res = program.at(program.at(pc + 1)) * program.at(program.at(pc + 2));
            pc += 4;
        } else {
            std::cerr << "UNKNOWN CMD " << program.at(pc) << " AT " << pc << std::endl;
            exit(1);
        }
    }
    return program.at(0);
}

int main(int argc, char* argv[])
{
    std::string line;
    std::getline(std::cin, line);

    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 1;
    }

    Program program;
    {
        std::vector<std::string> s;
        boost::algorithm::split(s, line, boost::algorithm::is_any_of(","));
        for (auto const& v : s) {
            program.push_back(std::stoi(v));
        }
    }

    {
        Program p1 = program;
        p1.at(1) = 12;
        p1.at(2) = 2;
        std::cout << "1: " << run_program(p1) << std::endl;
    }

    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            Program p2 = program;
            p2.at(1) = i;
            p2.at(2) = j;
            if (run_program(p2) == 19690720) {
                std::cout << "i: " << i << ", j: " << j << "\n";
                std::cout << "2: " << (100 * i + j) << std::endl;
                i = 100;
                break;
            }
        }
    }

    return 0;
}
