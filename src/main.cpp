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

TEST_CASE("[general] basic key-scalar") {
    std::string content = "key = value";
    std::shared_ptr<Object> object = ParseString(content);

    CHECK(object->GetEntries().size() == 1);
    CHECK(object->Get("key") != nullptr);
    CHECK(object->Get("key")->As<std::string>() == "value");
    CHECK(object->GetOperator("key") == Operator::EQUAL);
}