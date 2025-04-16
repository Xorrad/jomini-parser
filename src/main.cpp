#include <iostream>
#include <vector>

#include "Jomini.hpp"

using namespace Jomini;

int main() {
    std::cout << "hello world" << std::endl;

    // Scalar
    std::shared_ptr<Object> o = std::make_shared<Object>("test");
    std::cout << o->As<std::string>() << std::endl;

    // Array
    std::vector<std::shared_ptr<Object>> arr1;
    for (int i = 0; i < 10; i++)
        arr1.push_back(std::make_shared<Object>(std::to_string(i)));

    std::shared_ptr<Object> o1 = std::make_shared<Object>(arr1);
    
    for (auto v : o1->AsArray<double>())
        std::cout << v << " ";
    std::cout << std::endl;

    return 0;
}