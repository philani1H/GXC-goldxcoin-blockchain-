#include <iostream>
#include <string>
class CliInterface {
public:
    void parseCommand(const std::string& cmd) {
        std::cout << "Parsing command: " << cmd << std::endl;
    }
    void execute() {
        std::cout << "Executing CLI command..." << std::endl;
    }
};