#include <iostream>
#include <vector>

#include "Jomini.hpp"
using namespace Jomini;

// Dependencies for testing, debugging and benchmarking.
#include "backward/signal_handler.hpp"
SignalHandler signalHandler;

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.hpp"

int main(int argc, char** argv) {
    doctest::Context context(argc, argv);
    context.run();

    return 0;
}

TEST_CASE("[01_basic] basic key-op-scalar") {
    std::shared_ptr<Object> object = ParseFile("tests/01_basic.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetEntries().size() == 1);
    CHECK(object->Contains("key"));
    CHECK(object->Get("key") != nullptr);
    CHECK(object->Get("key")->GetType() == Type::SCALAR);
    CHECK(object->Get("key")->As<std::string>() == "value");
    CHECK(object->GetOperator("key") == Operator::EQUAL);
}

TEST_CASE("[02_basic_multi] multilines key-op-scalar") {
    std::shared_ptr<Object> object = ParseFile("tests/02_basic_multi.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetEntries().size() == 4);

    for (int i = 1; i < 5; i++) {
        std::string key = "key" + std::to_string(i);
        CHECK(object->Contains(key));
        CHECK(object->Get(key)->As<std::string>() == "value" + std::to_string(i));
        CHECK(object->GetOperator(key) == Operator::EQUAL);
    }
}

TEST_CASE("[03_operators] operators") {
    std::shared_ptr<Object> object = ParseFile("tests/03_operators.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetEntries().size() == 7);
    CHECK(object->GetOperator("equal") == Operator::EQUAL);
    CHECK(object->GetOperator("less") == Operator::LESS);
    CHECK(object->GetOperator("less_equal") == Operator::LESS_EQUAL);
    CHECK(object->GetOperator("greater") == Operator::GREATER);
    CHECK(object->GetOperator("greater_equal") == Operator::GREATER_EQUAL);
    CHECK(object->GetOperator("not_equal") == Operator::NOT_EQUAL);
    CHECK(object->GetOperator("not_null") == Operator::NOT_NULL);
}