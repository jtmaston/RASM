#include <iostream>
#include <fstream>
#include "Instruction.hpp"
#include <unordered_map>
#include <sstream>
#include <vector>
#include <iterator>

// TODO: syntax checking
// TODO: basically everything :)

std::unordered_map<std::string, uint8_t> human_readable_to_opcode =
{
    {"ANG", 0},
    {"ANGS", 1},
    {"DEL", 2},
    {"OFS", 3},
    {"NME", 4},
    {"SPD", 5},
    {"GHME", 6},
    {"SHME", 7},
    {"INC", 8},
    {"DEC", 9},
    {"RPP", 10},
    {"IPP", 11},
    {"END", 12},
    {"GTO", 13},
    {"IF", 14},
    {"IFN", 15},
    {"ABR", 16}
};

class Word : public std::string
{};

std::istream& operator>>(std::istream& is, Word& output)
{
   std::getline(is, output, ' ');
   return is;
}

int main( int argc, char **argv )
{

    if( argc == 1 ){
        std::cout << "rasm:\033[1;31m fatal error:\033[0m no input files \n";
        return 0;
    }

    std::ifstream input_file (argv[1]);

    std::vector<Instruction> instructions;
    uint32_t time = 0;
    uint64_t filesize = 0;

    for ( std::string line; getline(input_file, line);)
    {

        Instruction local;

        std::istringstream ss(line);
        std::vector<std::string> loc_args((std::istream_iterator<Word>(ss)),
                                 std::istream_iterator<Word>());
        local.opcode = human_readable_to_opcode[loc_args[0]];
        int i = 0;
        for(auto& arg: loc_args)
        {
            if ( arg == loc_args[0] )
                continue;

            if ( arg == "null" )
                local.params[i++] = -200;
            else
                local.params[i++] = std::stoi(arg);
        }
        instructions.push_back(local);
    }
    std::string output_file_name (argv[1]);
    size_t dot = output_file_name.find_last_of(".");
    output_file_name = output_file_name.substr(0, dot);
    output_file_name += ".bin";
    std::fstream output_file ( output_file_name , std::ios::out  | std::ios::binary);

    for( auto& instr : instructions){
        output_file.write((char*)&instr, sizeof(Instruction));
    }
    output_file.close();
}
