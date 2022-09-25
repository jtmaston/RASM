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
#include <regex>
#include "qrcodegen.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

// TODO: syntax checking
// TODO: basically everything :)
#define scaling_factor 32

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
        {"$", NUMERIC_VAR},
        {"@", STRING_VAR},
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
        {"EQ", EQ},
        {"TGT", TGT}};

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
    char arg[] = "DISPLAY=:0.0";
    putenv(arg);
    if (argc == 1)
    {
        std::cout << "rasm:\033[1;31m fatal error:\033[0m no input files \n";
        return 0;
    }

    std::vector<variable::Numeric> numeric_space; // holds numeric constants
    std::vector<variable::String> string_space;   // holds string constants
    std::vector<variable::Target> target_space;   // holds target constants

    std::string input_path = std::string(argv[1]);
    input_path.erase(std::remove(input_path.begin(), input_path.end(), '\\'), input_path.end());

    std::ifstream input_file(input_path);

    std::vector<Instruction> instructions;
    std::string output_file_name(argv[1]);
    size_t dot = output_file_name.find_last_of(".");
    output_file_name = output_file_name.substr(0, dot);

    size_t slash = output_file_name.find_last_of("/");
    //std::string progname = output_file_name.substr(slash + 1);
    std::string input_name = output_file_name.substr(slash + 1) + ".rasm";
    std::string progname;

    std::unordered_map<std::string, int> numeric_hashtable;
    std::unordered_map<std::string, int> goto_hashtable;
    std::unordered_map<std::string, int> target_hashtable;

    bool fail = false;
    int instruction_count = 0;

    for (std::string line; getline(input_file, line);)
    {
        instruction_count++;
        Instruction local;

        line = line.substr(0, line.find("//"));
        line = std::regex_replace(line, std::regex("^ +| +$|( ) +"), "$1");
        line = std::regex_replace(line, std::regex("/  +/g"), " ");

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
                try
                {
                    local.params[3] = goto_hashtable.at(loc_args[5]);
                }
                catch (std::exception E)
                {
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Use of undeclared label " << loc_args[5] << '\n';
                    fail = true;
                }
                break;
            }

            case NUMERIC_VAR:
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
                    fail = true;
                }
                continue;
            }
            case TGT:
            {
                variable::Target v;

                if (target_hashtable.find(loc_args[1]) == target_hashtable.end())
                {
                    target_hashtable.insert({loc_args[1], target_hashtable.size()});
                    v.name = target_hashtable.size() - 1;
                    target_space.push_back(v);
                }
                else
                {
                    v = target_space.at(numeric_hashtable.at(loc_args[1]));
                }

                local.params[0] = v.name;

                for (int i = 1; i <= 5; i++)
                    local.params[i] = std::stof(loc_args[i + 1]);

                break;
            }

                /*case TGT:
            {
                //std::cout << loc_args.size() << "\n";
                switch(loc_args.size())
                {
                    case 8:
                    {
                        std::vector<float> angles = {std::stof(loc_args[2]),
                            std::stof(loc_args[3]), std::stof(loc_args[4]),
                            std::stof(loc_args[5]), std::stof(loc_args[6]),
                            std::stof(loc_args[7])};
                        
                        target_hashtable.insert({std::move(loc_args[1]), std::move(angles)});
                        break;
                    }
                    default:
                    {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Not enough parameters to TGT function.\n";
                        fail = true;
                        break;
                    }
                }
                break;
            }*/

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

            case ANGS:
            {
                Instruction constant;
                switch (loc_args.size())
                {
                case 2:

                    local.params[0] = target_hashtable.at(loc_args[1]);
                    break;

                case 6:
                    
                    constant.opcode = 32;
                    target_hashtable.insert({std::string("tgt") + std::to_string(target_hashtable.size()), target_hashtable.size()});
                    constant.params[0] = target_hashtable.size() - 1;
                    for (int i = 1; i <= 5; i++)
                        constant.params[i] = std::stof(loc_args[i]);

                    instructions.push_back(constant);
                    local.params[0] = target_hashtable.size() - 1;

                    break;

                default:
                    std::cout << "rasm:\033[1;31m error:\033[0m " << input_name << ": " << instruction_count << ": "
                              << "Unknown overload of function ANGS ( called with " << loc_args.size() << " parameters )\n";
                    fail = true;
                    break;
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
        output_file_name = output_file_name.substr(0, output_file_name.find_last_of("/")) +"/"+ progname;
        std::string output_qr = output_file_name;
        output_qr += ".jpg";
        output_file_name += ".bin";
        std::fstream output_file(output_file_name, std::ios::out | std::ios::binary);

        output_file << progname << ' ';
        uint16_t checksum = 0;
        for (char const& c: progname)
        {
            checksum += c;
        }

        

        size_t progsize = instructions.size() * sizeof(Instruction);
        size_t numsize = numeric_space.size() * sizeof(variable::Numeric);
        size_t tgtsize = target_space.size() * sizeof(variable::Target);
        size_t strsize = 0;

        output_file << progsize << '\n';

        std::cout << "rasm: \033[0;32mDone!\n";
        std::cout << "\033[0;m" << progsize << " bytes used for program.\n";

        for (auto &instr : instructions)
        {
            output_file.write((char *)&instr, sizeof(Instruction));
        }
        output_file.close();

        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText((progname + " " + std::to_string(checksum)).c_str(), qrcodegen::QrCode::Ecc::HIGH);
        
        cv::Mat matrix(scaling_factor * qr.getSize(), scaling_factor * qr.getSize(), CV_8UC3, cv::Scalar(255, 255, 255));
        for( uint_fast16_t i = 0 ; i < qr.getSize(); i++ )
        {
            for(uint_fast16_t j = 0; j < qr.getSize(); j++)
            {
                if ( qr.getModule(i, j) )
                    cv::rectangle(matrix, cv::Point(i * scaling_factor, j * scaling_factor), cv::Point(i * scaling_factor + scaling_factor, j * scaling_factor + scaling_factor), cv::Scalar(0, 0, 0), cv::FILLED);
            }
        }
        cv::namedWindow("Output");
        cv::imshow("Output", matrix);
        cv::imwrite(output_qr, matrix);
        cv::waitKey(0);
        //cv::destroyWindow("Program output");

    }
    return 0;
}
