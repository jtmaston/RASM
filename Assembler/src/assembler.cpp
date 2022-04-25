#include <iostream>
#include <fstream>
#include "Instruction.hpp"
#include "Variable.hpp"
#include "isa.hpp"
#include <unordered_map>
#include <sstream>
#include <vector>
#include <iterator>
#include <exception>

// TODO: syntax checking
// TODO: basically everything :)

std::unordered_map<std::string, uint8_t> human_readable_to_opcode =
    {
        {"ANG", ANG},
        {"ANGS", ANGS},
        {"DEL", DEL},
        {"OFS", OFS},
        {"NME", NME},
        {"SPD", SPD},
        {"GHME", GHME},
        {"SHME", SHME},
        {"INC", INC},
        {"DEC", DEC},
        {"MOVJ", MOVJ},
        {"MOVL", MOVL},
        {"END", END},
        {"GOTO", GOTO},
        {"LABL", LABL},
        {"IF", IF},
        {"IFN", IFN},
        {"ABR", ABR},
        {"$", NUMERIC},
        {"@", STRING},
        {"ADD", ADD},
        {"PRT", PRT},
        {"SUB", SUB},
        {"DIV", DIV},
        {"FDIV", FDIV},
        {"SQRT", SQRT},
        {"TRNC", TRNC},
        {"LE", LE},
        {"L", L},
        {"GE", GE},
        {"G", G}, 
        {"EQ", EQ}
        };

class Word : public std::string
{
};

std::istream &operator>>(std::istream &is, Word &output)
{
    std::getline(is, output, ' ');
    return is;
}

int main(int argc, char **argv)
{

    if (argc == 1)
    {
        std::cout << "rasm:\033[1;31m fatal error:\033[0m no input files \n";
        return 0;
    }

    std::vector<variable::Numeric> numeric_space;
    std::vector<variable::String> string_space;

    std::ifstream input_file(argv[1]);

    std::vector<Instruction> instructions;
    std::string output_file_name(argv[1]);
    size_t dot = output_file_name.find_last_of(".");
    output_file_name = output_file_name.substr(0, dot);

    size_t slash = output_file_name.find_last_of("/");
    std::string progname = output_file_name.substr(slash + 1);
    std::string input_name = progname + ".rasm";

    std::unordered_map<std::string, int> numeric_hashtable;
    std::unordered_map<std::string, int> goto_hashtable;

    bool fail = false;
    int instruction_count = 0;

    for (std::string line; getline(input_file, line);)
    {
        instruction_count++;
        Instruction local;

        std::istringstream ss(line);
        std::vector<std::string> loc_args((std::istream_iterator<Word>(ss)),
                                          std::istream_iterator<Word>());

        if (line == "" || line == " " || line == "\t" || line == "\n")
            continue;

        if (human_readable_to_opcode.find(loc_args[0]) == human_readable_to_opcode.end())
        {
            std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                      << "Unrecognized instruction " << loc_args[0] << '\n';
            fail = true;
        }
        else
        {
            local.opcode = human_readable_to_opcode[loc_args[0]];

            switch (local.opcode)
            {
            case IF:
            {
                variable::Numeric v;
                if (numeric_hashtable.find(loc_args[1]) == numeric_hashtable.end())
                {
                    if (loc_args[1][0] == '#')
                    {
                        numeric_hashtable.insert({loc_args[1], numeric_hashtable.size()});
                        v.name = numeric_hashtable.size() - 1;
                        v.value = std::stoi(loc_args[1].substr(1));
                        numeric_space.push_back(v);
                    }
                    else
                    {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared variable for comparison. \n";
                        fail = true;
                        continue;
                    }                    
                }

                if (human_readable_to_opcode.find(loc_args[2]) == human_readable_to_opcode.end())
                {
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Unknown operator used in comparison. \n";
                    fail = true;
                    continue;
                }

                if (numeric_hashtable.find(loc_args[3]) == numeric_hashtable.end())
                {
                    if (loc_args[3][0] == '#')
                    {
                        numeric_hashtable.insert({loc_args[3], numeric_hashtable.size()});
                        v.name = numeric_hashtable.size() - 1;
                        v.value = std::stoi(loc_args[3].substr(1));
                        numeric_space.push_back(v);
                    }
                    else
                    {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared variable for comparison. \n";
                        fail = true;
                        continue;
                    }                    
                }

                if (loc_args[4] != "GOTO")
                {
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Missing GOTO designator!. \n";
                    fail = true;
                    continue;
                }

                local.params[0] = numeric_hashtable.at(loc_args[1]);
                local.params[1] = human_readable_to_opcode.at(loc_args[2]);
                local.params[2] = numeric_hashtable.at(loc_args[3]);
                //local.params[3] = std::stoi(loc_args[5]);                   // TODO: verification for this
                try {
                    local.params[3] = goto_hashtable.at(loc_args[5]);
                }catch(std::exception E)
                {
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared label " << loc_args[5] << '\n';
                    fail = true;
                }
                break;
            }

            case NUMERIC:
            {
                variable::Numeric v;
                if (numeric_hashtable.find(loc_args[1]) == numeric_hashtable.end())
                {
                    numeric_hashtable.insert({loc_args[1], numeric_hashtable.size()});
                    v.name = numeric_hashtable.size() - 1;
                    numeric_space.push_back(v);
                }
                else
                {
                    v = numeric_space.at(numeric_hashtable.at(loc_args[1]));
                }

                local.params[0] = v.name;
                local.params[1] = std::stoi(loc_args[2]);
                break;
            }

            case LABL:
            {
                if (goto_hashtable.find(loc_args[1]) == goto_hashtable.end())
                    goto_hashtable.insert({loc_args[1], instructions.size()});
                else
                {
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Duplicate GOTO label!. \n";
                }
                continue;
            }

            case ADD:
            case SUB:
            case DIV:
            case FDIV:
            {
                variable::Numeric v;
                if (numeric_hashtable.find(loc_args[1]) == numeric_hashtable.end())
                {
                    if (loc_args[1][0] == '#')
                    {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                                  << "Cannot use constant as output for operation \n";
                        fail = true;
                    }
                    else
                    {
                        std::cout << "rasm:\033[1;33m warning:\033[0m " << input_name << ": " << instruction_count << ": "
                                  << "Destination variable uninitialized, but written to. \n";
                        numeric_hashtable.insert({loc_args[1], numeric_hashtable.size()});
                        v.name = numeric_hashtable.size() - 1;
                        numeric_space.push_back(v);
                    }
                }

                if (numeric_hashtable.find(loc_args[2]) == numeric_hashtable.end())
                {
                    if (loc_args[2][0] == '#')
                    {
                        numeric_hashtable.insert({loc_args[2], numeric_hashtable.size()});
                        v.name = numeric_hashtable.size() - 1;
                        v.value = std::stoi(loc_args[2].substr(1));
                        numeric_space.push_back(v);
                    }
                    else
                    {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared variable for input. \n";
                        fail = true;
                        continue;
                    }                    
                }

                if (numeric_hashtable.find(loc_args[3]) == numeric_hashtable.end())
                {
                    if (loc_args[3][0] == '#')
                    {
                        numeric_hashtable.insert({loc_args[3], numeric_hashtable.size()});
                        v.name = numeric_hashtable.size() - 1;
                        v.value = std::stoi(loc_args[3].substr(1));
                        numeric_space.push_back(v);
                    }
                    else
                    {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared variable for input. \n";
                        fail = true;
                        continue;
                    }   
                }

                if (!fail)
                {
                    local.params[0] = numeric_hashtable.at(loc_args[1]);
                    local.params[1] = numeric_hashtable.at(loc_args[2]);
                    local.params[2] = numeric_hashtable.at(loc_args[3]);
                }

                break;
            }
            case SQRT:
            case TRNC:
            {

                if (numeric_hashtable.find(loc_args[1]) == numeric_hashtable.end())
                {
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared variable for input. \n";
                    continue;
                }

                if (!fail)
                {
                    local.params[0] = numeric_hashtable.at(loc_args[1]);
                }

                break;
            }
            default:
            {
                int i = 0;

                if (loc_args[0] == "NME")
                {
                    progname = loc_args[1];
                    continue;
                }
                for (auto &arg : loc_args)
                {
                    if (arg == loc_args[0])
                        continue;

                    if (arg == "null")
                        local.params[i++] = -200;
                    else
                    {
                        //std::cout << loc_args[0] << " ";
                        //std::cout << arg << '\n';
                        try
                        {
                            local.params[i++] = std::stoi(arg);
                        }
                        catch (std::exception E)
                        {
                            //std::cout << i << '\n';
                            local.params[i++] = numeric_hashtable.at(arg);
                        }
                    }
                }
            }
            }
            instructions.push_back(local);
        }
    }

    if (!fail)
    {
        output_file_name += ".bin";
        std::fstream output_file(output_file_name, std::ios::out | std::ios::binary);

        output_file << progname << ' ';

        size_t progsize = instructions.size() * sizeof(Instruction);
        size_t numsize = numeric_space.size() * sizeof(variable::Numeric);
        size_t strsize = 0;

        output_file << progsize << " " << numsize << " " << strsize << '\n';

        for (auto &str : string_space)
        {
            strsize += sizeof(variable::String);
            strsize += sizeof(char) * str.size;
        }

        std::cout << "rasm: \033[0;32mDone!\n";
        std::cout << "\033[0;m" << progsize << " bytes used for program.\n";
        std::cout << numsize << " bytes used for variable space.\n";
        std::cout << strsize << " bytes used for strings.\n";

        for (auto &instr : instructions)
        {
            output_file.write((char *)&instr, sizeof(Instruction));
        }

        for (auto &num : numeric_space)
        {
            output_file.write((char *)&num, sizeof(variable::Numeric));
        }

        output_file.close();
    }
    return 0;
}
