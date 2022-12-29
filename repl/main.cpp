#include <iostream>
#include "../scheme.h"

int main() {
    Interpreter interpreter;
    std::cout << "(pseudo)Scheme Language interpreter by @pepilica, 2022" << std::endl;
    std::cout << "Type \"exit\" to exit" << std::endl;
    std::string cur_string;
    std::string result;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, cur_string);
        if (cur_string == "exit") {
            break;
        }
        try {
            std::cout << interpreter.Run(cur_string) << std::endl;
            std::cerr.flush();
        } catch (SyntaxError&) {
            std::cerr << "Syntax error occurred!" << std::endl;
            std::cerr.flush();
        } catch (NameError&) {
            std::cerr << "Name error occurred!" << std::endl;
            std::cerr.flush();
        } catch (RuntimeError&) {
            std::cerr << "Runtime error occurred!" << std::endl;
            std::cerr.flush();
        } catch (...) {
            std::cerr << "Some internal error occurred! Exiting..." << std::endl;
            std::cerr.flush();
            throw;
        }
    }
    return 0;
}
