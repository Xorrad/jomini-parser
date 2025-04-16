#include <iostream>
#include <vector>

#include "debug/SignalHandler.hpp"
#include "Jomini.hpp"

using namespace Jomini;

SignalHandler signalHandler;

int main() {
    // Scalar
    std::shared_ptr<Object> o = std::make_shared<Object>("test");
    std::cout << o->As<std::string>() << std::endl << std::endl;

    // Array
    std::vector<std::shared_ptr<Object>> arr;
    for (int i = 0; i < 10; i++)
        arr.push_back(std::make_shared<Object>(std::to_string(i)));

    std::shared_ptr<Object> o1 = std::make_shared<Object>(arr);

    for (auto v : o1->AsArray<double>())
        std::cout << v << " ";
    std::cout << std::endl << std::endl;

    // Object
    OrderedMap<std::string, std::pair<Operator, std::shared_ptr<Object>>> entries;
    for (int i = 9; i > 0; i--)
        entries.insert(std::to_string(i), std::make_pair(Operator::EQUAL, std::make_shared<Object>(std::to_string(i))));
    entries.insert("test", std::make_pair(Operator::EQUAL, std::make_shared<Object>("test")));
    std::shared_ptr<Object> o2 = std::make_shared<Object>(entries);

    for (auto [key, pair] : o2->GetEntries())
        std::cout << key << " = " << pair.second->As<std::string>() << std::endl;
    std::cout << std::endl;

    std::cout << "test" << " = " << o2->Get("test")->As<std::string>() << std::endl;

    return 0;
}