#include <iostream>
#include "Jomini.hpp"

using namespace Jomini;

int main() {
    std::cout << "hello world" << std::endl;

    std::shared_ptr<Object> o = std::make_shared<Object>("test");

    std::cout << o->As<std::string>() << std::endl;

    return 0;
}