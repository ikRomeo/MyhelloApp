
#include "First_App.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    ikE::FirstApp app{};

    try {
        app.run();
    }            
     
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}