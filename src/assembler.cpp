#include <iostream>
#include <fstream>
#include "Instruction.hpp"
#include "Variable.hpp"
#include "isa.hpp"
#include <unordered_map>
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
#define SCALING_FACTOR 32

std::unordered_map<std::string, uint8_t> humanReadableToOpcode =
        {
                {"ANG",  ANG},
                {"ANGS", ANGS},
                {"DEL",  DEL},
                {"OFS",  OFS},
                {"NME",  NME},
                {"SPD",  SPD},
                {"GHME", GHME},
                {"SHME", SHME},
                {"INC",  INC},
                {"DEC",  DEC},
                {"MOVJ", MOVJ},
                {"MOVL", MOVL},
                {"END",  END},
                {"GOTO", GOTO},
                {"LABL", LABL},
                {"IF",   IF},
                {"IFN",  IFN},
                {"ABR",  ABR},
                {"$",    NUMERIC_VAR},
                {"@",    STRING_VAR},
                {"ADD",  ADD},
                {"PRT",  PRT},
                {"SUB",  SUB},
                {"DIV",  DIV},
                {"FDIV", FDIV},
                {"SQRT", SQRT},
                {"TRNC", TRNC},
                {"LE",   LE},
                {"L",    L},
                {"GE",   GE},
                {"G",    G},
                {"EQ",   EQ},
                {"TGT",  TGT}};

class word : public std::string {
};

std::istream &operator>>(std::istream &is, word &output) {
    std::getline(is, output, ' ');
    return is;
}

int main(int argc, char **argv) {
    char arg[] = "DISPLAY=:0.0";
    putenv(arg);
    if (argc == 1) {
        std::cout << "rasm:\033[1;31m fatal error:\033[0m no input files \n";
        return 0;
    }

    std::vector<variable::Numeric> numericSpace; // holds numeric constants
    std::vector<variable::String> stringSpace;   // holds string constants
    std::vector<variable::Target> targetSpace;   // holds target constants

    std::string inputPath = std::string(argv[1]);
    inputPath.erase(std::remove(inputPath.begin(), inputPath.end(), '\\'), inputPath.end());

    std::ifstream inputFile(inputPath);

    std::vector<Instruction> instructions;
    std::string outputFileName(argv[1]);
    size_t dot = outputFileName.find_last_of('.');
    outputFileName = outputFileName.substr(0, dot);
    std::string programName = "placeholder";

    size_t slash = outputFileName.find_last_of('/');
    std::string inputName = outputFileName.substr(slash + 1) + ".rasm";


    std::unordered_map<std::string, int> numericHashtable;
    std::unordered_map<std::string, int> gotoHashtable;
    std::unordered_map<std::string, int> targetHashtable;

    bool fail = false;
    int instructionCount = 0;


    for (std::string line; getline(inputFile, line);) {
        instructionCount++;
        Instruction local;

        line = line.substr(0, line.find("//"));
        line = std::regex_replace(line, std::regex("^ +| +$|( ) +"), "$1");
        line = std::regex_replace(line, std::regex("/  +/g"), " ");

        std::istringstream ss(line);
        std::vector<std::string> locArgs((std::istream_iterator<word>(ss)),
                                         std::istream_iterator<word>());

        if (line.empty() || line == " " || line == "\t" || line == "\n")
            continue;

        if (humanReadableToOpcode.find(locArgs[0]) == humanReadableToOpcode.end()) {
            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount << ": "
                      << "Unrecognized instruction " << locArgs[0] << '\n';
            fail = true;
        } else {
            local.opcode = humanReadableToOpcode[locArgs[0]];

            switch (local.opcode) {
                case IF: {
                    variable::Numeric v;
                    if (numericHashtable.find(locArgs[1]) == numericHashtable.end()) {
                        if (locArgs[1][0] == '#') {
                            numericHashtable.insert({locArgs[1], numericHashtable.size()});
                            v.name = numericHashtable.size() - 1;
                            v.value = std::stoi(locArgs[1].substr(1));
                            numericSpace.push_back(v);
                        } else {
                            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Use of undeclared variable for comparison. \n";
                            fail = true;
                            continue;
                        }
                    }

                    if (humanReadableToOpcode.find(locArgs[2]) == humanReadableToOpcode.end()) {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount << ": "
                                  << "Unknown operator used in comparison. \n";
                        fail = true;
                        continue;
                    }

                    if (numericHashtable.find(locArgs[3]) == numericHashtable.end()) {
                        if (locArgs[3][0] == '#') {
                            numericHashtable.insert({locArgs[3], numericHashtable.size()});
                            v.name = numericHashtable.size() - 1;
                            v.value = std::stoi(locArgs[3].substr(1));
                            numericSpace.push_back(v);
                        } else {
                            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Use of undeclared variable for comparison. \n";
                            fail = true;
                            continue;
                        }
                    }

                    if (locArgs[4] != "GOTO") {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount << ": "
                                  << "Missing GOTO designator!. \n";
                        fail = true;
                        continue;
                    }

                    local.params.at(0) = (float) numericHashtable.at(locArgs[1]);
                    local.params.at(1) = (float) humanReadableToOpcode.at(locArgs[2]);
                    local.params.at(2) = (float) numericHashtable.at(locArgs[3]);
                    //local.params[3] = std::stoi(locArgs[5]);                   // TODO: verification for this
                    try {
                        local.params[3] = (float) gotoHashtable.at(locArgs[5]);
                    }
                    catch (std::exception &E) {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount << ": "
                                  << "Use of undeclared label " << locArgs[5] << '\n';
                        fail = true;
                    }
                    break;
                }

                case NUMERIC_VAR: {
                    static variable::Numeric v;
                    if (numericHashtable.find(locArgs[1]) == numericHashtable.end()) {
                        numericHashtable.insert({locArgs[1], numericHashtable.size()});
                        v.name = numericHashtable.size() - 1;
                        numericSpace.push_back(v);
                    } else {
                        v = numericSpace.at(numericHashtable.at(locArgs[1]));
                    }

                    local.params[0] = (float) v.name;
                    local.params[1] = (float) std::stoi(locArgs[2]);
                    break;
                }

                case LABL: {
                    if (gotoHashtable.find(locArgs[1]) == gotoHashtable.end())
                        gotoHashtable.insert({locArgs[1], instructions.size()});
                    else {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount << ": "
                                  << "Duplicate GOTO label!. \n";
                        fail = true;
                    }
                    continue;
                }
                case TGT: {
                    static variable::Target v;

                    if (targetHashtable.find(locArgs[1]) == targetHashtable.end()) {
                        targetHashtable.insert({locArgs[1], targetHashtable.size()});
                        v.name = targetHashtable.size() - 1;
                        targetSpace.push_back(v);
                    } else {
                        v = targetSpace.at(numericHashtable.at(locArgs[1]));
                    }

                    local.params[0] = v.name;

                    for (int i = 1; i <= 5; i++)
                        local.params[i] = std::stof(locArgs[i + 1]);

                    break;
                }

                case ADD:
                case SUB:
                case DIV:
                case FDIV: {
                    variable::Numeric v;
                    if (numericHashtable.find(locArgs[1]) == numericHashtable.end()) {
                        if (locArgs[1][0] == '#') {
                            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Cannot use constant as output for operation \n";
                            fail = true;
                        } else {
                            std::cout << "rasm:\033[1;33m warning:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Destination variable uninitialized, but written to. \n";
                            numericHashtable.insert({locArgs[1], numericHashtable.size()});
                            v.name = numericHashtable.size() - 1;
                            numericSpace.push_back(v);
                        }
                    }

                    if (numericHashtable.find(locArgs[2]) == numericHashtable.end()) {
                        if (locArgs[2][0] == '#') {
                            numericHashtable.insert({locArgs[2], numericHashtable.size()});
                            v.name = numericHashtable.size() - 1;
                            v.value = std::stoi(locArgs[2].substr(1));
                            numericSpace.push_back(v);
                        } else {
                            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Use of undeclared variable for input. \n";
                            fail = true;
                            continue;
                        }
                    }

                    if (numericHashtable.find(locArgs[3]) == numericHashtable.end()) {
                        if (locArgs[3][0] == '#') {
                            numericHashtable.insert({locArgs[3], numericHashtable.size()});
                            v.name = numericHashtable.size() - 1;
                            v.value = std::stoi(locArgs[3].substr(1));
                            numericSpace.push_back(v);
                        } else {
                            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Use of undeclared variable for input. \n";
                            fail = true;
                            continue;
                        }
                    }

                    if (!fail) {
                        local.params.at(0) = (float) numericHashtable.at(locArgs[1]);
                        local.params.at(1) = (float) numericHashtable.at(locArgs[2]);
                        local.params.at(2) = (float) numericHashtable.at(locArgs[3]);
                    }

                    break;
                }
                case SQRT:
                case TRNC: {

                    if (numericHashtable.find(locArgs[1]) == numericHashtable.end()) {
                        std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount << ": "
                                  << "Use of undeclared variable for input. \n";
                        continue;
                    }

                    if (!fail) {
                        local.params.at(0) = (float) numericHashtable.at(locArgs[1]);
                    }

                    break;
                }

                case ANGS: {
                    Instruction constant;
                    switch (locArgs.size()) {
                        case 2:

                            local.params.at(0) = (float) targetHashtable.at(locArgs[1]);
                            break;

                        case 6:

                            constant.opcode = 32;
                            targetHashtable.insert({std::string("tgt") + std::to_string(targetHashtable.size()),
                                                    targetHashtable.size()});
                            constant.params.at(0) = (float) targetHashtable.size() - 1;
                            for (int i = 1; i <= 5; i++)
                                constant.params[i] = std::stof(locArgs[i]);

                            instructions.push_back(constant);
                            local.params.at(0) = (float) (targetHashtable.size() - 1);

                            break;

                        default:
                            std::cout << "rasm:\033[1;31m error:\033[0m " << inputName << ": " << instructionCount
                                      << ": "
                                      << "Unknown overload of function ANGS ( called with " << locArgs.size()
                                      << " parameters )\n";
                            fail = true;
                            break;
                    }

                    break;
                }

                default: {
                    int i = 0;

                    if (locArgs[0] == "NME") {
                        programName = std::move(locArgs[1]);
                        programName.clear();

                        continue;
                    }
                    for (auto &localArgument: locArgs) {
                        if (localArgument == locArgs[0])
                            continue;

                        if (localArgument == "null")
                            local.params[i++] = -200;
                        else {
                            try {
                                local.params.at(i++) = (float) (std::stoi(localArgument));
                            }
                            catch (std::exception &E) {
                                local.params.at(i++) = (float) (numericHashtable.at(localArgument));
                            }
                        }
                    }
                }
            }
            instructions.push_back(local);
        }
    }

    if (!fail) {
        outputFileName = outputFileName.substr(0, outputFileName.find_last_of('/')) + "/" + programName;
        std::string outputQr = outputFileName;
        outputQr += ".jpg";
        outputFileName += ".bin";
        std::fstream outputFile(outputFileName, std::ios::out | std::ios::binary);

        outputFile << programName << ' ';
        uint16_t checksum = 0;
        for (char const &c: programName) {
            checksum += c;
        }


        size_t programSize = instructions.size() * sizeof(Instruction);

        outputFile << programSize << '\n';

        std::cout << "rasm: \033[0;32mDone!\n";
        std::cout << "\033[0;m" << programSize << " bytes used for program.\n";

        for (auto &instr: instructions) {
            outputFile.write((char *) &instr, sizeof(Instruction));
        }
        outputFile.close();

        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText((programName + " " + std::to_string(checksum)).c_str(),
                                                             qrcodegen::QrCode::Ecc::HIGH);

        cv::Mat matrix(SCALING_FACTOR * qr.getSize(), SCALING_FACTOR * qr.getSize(), CV_8UC3,
                       cv::Scalar(255, 255, 255));
        for (int i = 0; i < qr.getSize(); i++) {
            for (int j = 0; j < qr.getSize(); j++) {
                if (qr.getModule(i, j))
                    cv::rectangle(matrix, cv::Point(i * SCALING_FACTOR, j * SCALING_FACTOR),
                                  cv::Point(i * SCALING_FACTOR + SCALING_FACTOR, j * SCALING_FACTOR + SCALING_FACTOR),
                                  cv::Scalar(0, 0, 0), cv::FILLED);
            }
        }
        cv::namedWindow("Output");
        cv::imshow("Output", matrix);
        cv::imwrite(outputQr, matrix);
        cv::waitKey(0);
        //cv::destroyWindow("Program output");

    }
    return 0;
}
