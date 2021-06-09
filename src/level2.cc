#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    std::vector<unsigned> program;

    std::string line;
    std::getline(std::cin, line);

    if (line.empty()) {
        std::cout << "NO INPUT" << std::endl;
        return 1;
    }

    {
        std::vector<std::string> s;
        boost::algorithm::split(s, line, boost::algorithm::is_any_of(","));
        for(auto const& v : s) {
            program.push_back(std::stoi(v));
        }
    }

    program.at(1) = 12;
    program.at(2) = 2;

    for (unsigned pc = 0;;)
    {
        // std::cout << "PC: " << pc << "; [";
        // for (auto const& i : program)
        // {
        //     std::cout << i << ",";
        // }
        // std::cout << "]";

        if (program.at(pc) == 99)
        {
            break;
        }
        else if (program.at(pc) == 1)
        {
            auto& res = program.at(program.at(pc + 3));
            res       = program.at(program.at(pc + 1)) + program.at(program.at(pc + 2));
            pc += 4;
        }
        else if (program.at(pc) == 2)
        {
            auto& res = program.at(program.at(pc + 3));
            res       = program.at(program.at(pc + 1)) * program.at(program.at(pc + 2));
            pc += 4;
        }
        else
        {
            std::cerr << "UNKNOWN CMD " << program.at(pc) << " AT " << pc << std::endl;
            exit(1);
        }

        // std::cout << " --> [";
        // for (auto const& i : program)
        // {
        //     std::cout << i << ",";
        // }
        // std::cout << "]" << std::endl;
    }
    // std::cout << std::endl;

    std::cout << "1: " << program.at(0) << std::endl;

    return 0;
}
