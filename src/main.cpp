#include <iostream>
#include <chrono>
#include <iomanip>
#include <regex>

#include "Jomini.hpp"
using namespace Jomini;

// Dependencies for testing, debugging and benchmarking.
#include "backward/signal_handler.hpp"
SignalHandler signalHandler;

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.hpp"

// Function to test the parser manually.
void ManualTests();

// Function to measure reading and parsing speed.
void Benchmark();

int main(int argc, char** argv) {
    doctest::Context context(argc, argv);
    context.run();

    ManualTests();
    Benchmark();

    return 0;
}

void ManualTests() {
    std::shared_ptr<Object> object = ParseFile("tests/00_tests.txt");

    std::cout << "keys:" << std::endl;
    for (auto [key, pair] : object->GetMap()) {
        std::cout << key << std::endl;
    }
    std::cout << std::endl;
}

void Benchmark() {
    struct BenchmarkResult {
        std::string filePath;
        std::chrono::duration<double, std::milli> duration;
        double entries;
    };

    const auto BenchmarkFile = [](const std::string& filePath, uint iterations) {
        BenchmarkResult result = {
            filePath,
            std::chrono::duration<double, std::milli>::zero(),
            0 
        };

        for (int i = 0; i < iterations; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            std::shared_ptr<Object> data = ParseFile(filePath);
            auto end = std::chrono::high_resolution_clock::now();
            result.duration += end - start;
            result.entries += data->GetMap().size();
        }

        result.filePath = filePath;
        result.duration /= iterations;
        result.entries /= iterations;
        return result;
    };

    std::cout << "Starting benchmarks..." << std::endl;
    
    std::vector<BenchmarkResult> results;
    results.push_back(BenchmarkFile("tests/00_benchmark_10KB.txt", 10));
    results.push_back(BenchmarkFile("tests/00_benchmark_100KB.txt", 5));
    results.push_back(BenchmarkFile("tests/00_benchmark_1MB.txt", 5));
    results.push_back(BenchmarkFile("tests/00_tests.txt", 1));

    std::cout << std::left << std::setw(30) << "file path" << std::right << std::setw(15) << "avg time" << std::setw(15) << "avg entries" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    
    for (auto result : results) {
        std::cout << std::left << std::setw(30) << result.filePath << std::right << std::setw(15) << (std::to_string(result.duration.count()) + "ms") << std::setw(15) << result.entries << std::endl;
    }
}

std::string SerializeVector(const std::vector<std::string>& vec) {
    std::string str = "{";
    for (int i = 0; i < vec.size(); i++)
        str += " " + vec[i] + (i == vec.size()-1 ? " " : "");
    return str + "}";
}

TEST_CASE("[00_empty] empty file") {
    std::shared_ptr<Object> object = ParseFile("tests/00_empty.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetMap().size() == 0);
}

TEST_CASE("[01_basic] basic key-op-scalar") {
    std::shared_ptr<Object> object = ParseFile("tests/01_basic.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetMap().size() == 1);
    CHECK(object->Contains("key"));
    CHECK(object->Get("key") != nullptr);
    CHECK(object->Get("key")->GetType() == Type::SCALAR);
    CHECK(object->Get("key")->As<std::string>() == "value");
    CHECK(object->GetOperator("key") == Operator::EQUAL);
}

TEST_CASE("[02_basic_multi] multilines key-op-scalar") {
    std::shared_ptr<Object> object = ParseFile("tests/02_basic_multi.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetMap().size() == 4);

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
    CHECK(object->GetMap().size() == 7);
    CHECK(object->GetOperator("equal") == Operator::EQUAL);
    CHECK(object->GetOperator("less") == Operator::LESS);
    CHECK(object->GetOperator("less_equal") == Operator::LESS_EQUAL);
    CHECK(object->GetOperator("greater") == Operator::GREATER);
    CHECK(object->GetOperator("greater_equal") == Operator::GREATER_EQUAL);
    CHECK(object->GetOperator("not_equal") == Operator::NOT_EQUAL);
    CHECK(object->GetOperator("not_null") == Operator::NOT_NULL);
}

TEST_CASE("[04_nested_object] nested objects") {
    std::shared_ptr<Object> object = ParseFile("tests/04_nested_objects.txt");

    CHECK(object->GetType() == Type::OBJECT);
    CHECK(object->GetMap().size() == 1);

    CHECK(object->Contains("key1"));
    CHECK(object->Get("key1")->GetType() == Type::OBJECT);
    CHECK(object->Get("key1")->GetMap().size() == 2);
    
    CHECK(object->Get("key1")->Contains("key2"));
    CHECK(object->Get("key1")->Get("key2")->GetType() == Type::OBJECT);
    CHECK(object->Get("key1")->Get("key2")->GetMap().size() == 1);
    CHECK(object->Get("key1")->Get("key2")->Contains("key3"));
    CHECK(object->Get("key1")->Get("key2")->Get("key3")->GetType() == Type::SCALAR);
    CHECK(object->Get("key1")->Get("key2")->Get("key3")->As<std::string>() == "value3");
    
    CHECK(object->Get("key1")->Contains("key4"));
    CHECK(object->Get("key1")->Get("key4")->GetType() == Type::SCALAR);
    CHECK(object->Get("key1")->Get("key4")->As<std::string>() == "value4");
}

TEST_CASE("[05_scalars] scalars") {
    std::shared_ptr<Object> object = ParseFile("tests/05_scalars.txt");

    CHECK(object->Get("int")->As<int>() == 12);
    CHECK(object->Get("int_negative")->As<int>() == -12);
    CHECK(object->Get("double")->As<double>() == 12.25);
    CHECK(object->Get("double_negative")->As<double>() == -12.25);
    CHECK(object->Get("bool_true")->As<bool>());
    CHECK_FALSE(object->Get("bool_false")->As<bool>());
    CHECK(object->Get("string")->As<std::string>() == "abcdefghijklmnopqrstuvwxyz");
    CHECK(object->Get("string_literal")->As<std::string>() == "\"Hello World!\\n\"");
    CHECK(object->Get("date")->As<Date>() == Date(2025, 1, 14));
}

TEST_CASE("[06_keys] keys") {
    std::shared_ptr<Object> object = ParseFile("tests/06_keys.txt");

    CHECK(object->Get("12")->As<std::string>() == "int");
    CHECK(object->Get("-12")->As<std::string>() == "int_negative");
    CHECK(object->Get("12.25")->As<std::string>() == "double");
    CHECK(object->Get("-12.25")->As<std::string>() == "double_negative");
    CHECK(object->Get("yes")->As<std::string>() == "bool_true");
    CHECK(object->Get("no")->As<std::string>() == "bool_false");
    CHECK(object->Get("abcdefghijklmnopqrstuvwxyz")->As<std::string>() == "string");
    CHECK(object->Get("\"Hello World!\\n\"")->As<std::string>() == "string_literal");
    CHECK(object->Get("2025.1.14")->As<std::string>() == "date");
}

TEST_CASE("[07_keys_ordering] keys ordering once parsed") {
    std::shared_ptr<Object> object = ParseFile("tests/07_keys_ordering.txt");

    CHECK(object->GetMap().keys().at(0) == "key1");
    CHECK(object->GetMap().keys().at(1) == "key2");
    CHECK(object->GetMap().keys().at(2) == "key0");
    CHECK(object->GetMap().keys().at(3) == "key3");
    CHECK(object->Get("key3")->GetMap().keys().at(0) == "key2");
    CHECK(object->Get("key3")->GetMap().keys().at(1) == "key1");
}

TEST_CASE("[08_arrays_basic] basic arrays") {
    std::shared_ptr<Object> object = ParseFile("tests/08_arrays_basic.txt");

    CHECK(object->Get("key1")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key1")->AsArray<std::string>()) == "{ v1 }");
    CHECK(object->Get("key2")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key2")->AsArray<std::string>()) == "{ v1 v2 }");
    CHECK(object->Get("key3")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key3")->AsArray<std::string>()) == "{ v1 v2 v3 }");
    CHECK(object->Get("key4")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key4")->AsArray<std::string>()) == "{ 10 }");
    CHECK(object->Get("key5")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key5")->AsArray<std::string>()) == "{ 10 20 }");
    CHECK(object->Get("key6")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key6")->AsArray<std::string>()) == "{ 10 20 30 }");
}

TEST_CASE("[09_arrays_complex] complex arrays") {
    std::shared_ptr<Object> object = ParseFile("tests/09_arrays_complex.txt");

    CHECK(object->Get("key1")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key1")->AsArray<std::string>()) == "{ culture:roman culture:breton }");
    CHECK(object->Get("key2")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key2")->AsArray<std::string>()) == "{ 10 -10 10.05 -10.05 yes no value1 \"Maximus Decimus Meridius\" 2025.10.31 }");
    CHECK(object->Get("key3")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key3")->AsArray<std::string>()) == "{ @var 100 }");
    
    CHECK(object->Get("key4")->GetType() == Type::ARRAY);
    CHECK(object->Get("key4")->GetArray().size() == 1);
    CHECK(object->Get("key4")->GetArray().at(0)->Get("name")->As<std::string>() == "\"Claudius\"");

    CHECK(object->Get("key5")->GetType() == Type::ARRAY);
    CHECK(object->Get("key5")->GetArray().size() == 3);
    CHECK(object->Get("key5")->GetArray().at(0)->Get("name")->As<std::string>() == "\"Julius\"");
    CHECK(object->Get("key5")->GetArray().at(1)->Get("name")->As<std::string>() == "\"Gaius\"");
    CHECK(object->Get("key5")->GetArray().at(2)->Get("name")->As<std::string>() == "\"Maximus\"");

    CHECK(object->Get("key6")->GetType() == Type::ARRAY);
    CHECK(object->Get("key6")->GetArray().size() == 3);
    CHECK(object->Get("key6")->GetArray().at(0)->As<std::string>() == "10");
    CHECK(object->Get("key6")->GetArray().at(1)->Get("name")->As<std::string>() == "\"Aurelius\"");
    CHECK(object->Get("key6")->GetArray().at(2)->As<std::string>() == "culture:greek");
    
    CHECK(object->Get("key7")->GetType() == Type::ARRAY);
    CHECK(object->Get("key7")->GetArray().size() == 2);
    CHECK(object->Get("key7")->GetArray().at(0)->As<std::string>() == "2025.6.2");
    CHECK(object->Get("key7")->GetArray().at(1)->Get("name")->As<std::string>() == "\"Vespasian\"");
}

TEST_CASE("[10_arrays_concatenation] concatenation of arrays") {
    std::shared_ptr<Object> object = ParseFile("tests/10_arrays_concatenation.txt");

    CHECK(object->Get("trait")->GetType() == Type::ARRAY);
    CHECK(object->Get("trait")->GetArray().size() == 4);
    CHECK(SerializeVector(object->Get("trait")->AsArray<std::string>()) == "{ education_stewardship_4 patient zealous diligent }");
}

TEST_CASE("[11_arrays_flags] arrays with special flags (color, range...)") {
    std::shared_ptr<Object> object = ParseFile("tests/11_arrays_flags.txt");

    CHECK(object->Get("color1")->GetType() == Type::ARRAY);
    CHECK(object->Get("color1")->GetFlags() == Flags::RGB);
    CHECK(object->Get("color2")->GetType() == Type::ARRAY);
    CHECK(object->Get("color2")->GetFlags() == Flags::HSV);
    
    CHECK(object->Get("list1")->GetType() == Type::ARRAY);
    CHECK(object->Get("list1")->GetFlags() == Flags::LIST);
    CHECK(object->Get("list1")->GetArray().empty());
    CHECK(object->Get("list2")->GetType() == Type::ARRAY);
    CHECK(object->Get("list2")->GetFlags() == Flags::LIST);
    CHECK(SerializeVector(object->Get("list2")->AsArray<std::string>()) == "{ 2 1 3 4 }");
    
    CHECK(object->Get("range1")->GetType() == Type::ARRAY);
    CHECK(object->Get("range1")->GetFlags() == Flags::RANGE);
    CHECK(SerializeVector(object->Get("range1")->AsArray<std::string>()) == "{ 1 2 3 4 5 }");
    CHECK(object->Get("range2")->GetType() == Type::ARRAY);
    CHECK(object->Get("range2")->GetFlags() == Flags::RANGE);
    CHECK(SerializeVector(object->Get("range2")->AsArray<std::string>()) == "{ 5 4 3 2 1 }");
    CHECK(object->Get("range3")->GetType() == Type::ARRAY);
    CHECK(object->Get("range3")->GetFlags() == (Flags::RANGE | Flags::LIST));
    for (auto o : object->Get("range3")->GetArray())
        CHECK(o->GetType() == Type::SCALAR);
    CHECK(SerializeVector(object->Get("range3")->AsArray<std::string>()) == "{ 1 2 3 4 10 2 }");
}

TEST_CASE("[12_comments] comments") {
    std::shared_ptr<Object> object = ParseFile("tests/12_comments.txt");

    CHECK(object->Get("key1")->GetType() == Type::SCALAR);
    CHECK(object->Get("key1")->As<std::string>() == "value1");
    CHECK(object->Get("key2")->GetType() == Type::SCALAR);
    CHECK(object->Get("key2")->As<std::string>() == "value2");
    CHECK(object->Get("key3")->GetType() == Type::OBJECT);
    CHECK(object->Get("key3")->GetMap().empty());
    CHECK(object->Get("key4")->GetType() == Type::ARRAY);
    CHECK(SerializeVector(object->Get("key4")->AsArray<std::string>()) == "{ v1 v2 }");
}

TEST_CASE("[13_utf8] utf8 characters") {
    std::shared_ptr<Object> object = ParseFile("tests/13_utf8.txt");

    CHECK(object->Get("latin1")->GetType() == Type::SCALAR);
    CHECK(object->Get("latin1")->As<std::string>() == "éàâêëïîôöäüûù");
    CHECK(object->Get("éàâêëïîôöäüûù")->GetType() == Type::SCALAR);
    CHECK(object->Get("éàâêëïîôöäüûù")->As<std::string>() == "latin2");
    
    CHECK(object->Get("cyrillic1")->GetType() == Type::SCALAR);
    CHECK(object->Get("cyrillic1")->As<std::string>() == "\"бутерброд\"");
    CHECK(object->Get("cyrillic2")->GetType() == Type::SCALAR);
    CHECK(object->Get("cyrillic2")->As<std::string>() == "ничто");
    CHECK(object->Get("бутерброд")->GetType() == Type::SCALAR);
    CHECK(object->Get("бутерброд")->As<std::string>() == "cyrillic3");

    CHECK(object->Get("japanese1")->GetType() == Type::SCALAR);
    CHECK(object->Get("japanese1")->As<std::string>() == "\"やすむ\"");
    CHECK(object->Get("japanese2")->GetType() == Type::SCALAR);
    CHECK(object->Get("japanese2")->As<std::string>() == "カタツムリ");
    CHECK(object->Get("japanese3")->GetType() == Type::SCALAR);
    CHECK(object->Get("japanese3")->As<std::string>() == "蝸牛");
    CHECK(object->Get("蝸牛")->GetType() == Type::SCALAR);
    CHECK(object->Get("蝸牛")->As<std::string>() == "japanese4");

    CHECK(object->Get("chinese1")->GetType() == Type::SCALAR);
    CHECK(object->Get("chinese1")->As<std::string>() == "\"重要\"");
    CHECK(object->Get("chinese2")->GetType() == Type::SCALAR);
    CHECK(object->Get("chinese2")->As<std::string>() == "眼泪");
    CHECK(object->Get("眼泪")->GetType() == Type::SCALAR);
    CHECK(object->Get("眼泪")->As<std::string>() == "chinese3");
}

TEST_CASE("[14_exceptions_missing_value] value missing after key and operator") {
    CHECK_THROWS_AS(ParseFile("tests/14_exceptions_missing_value.txt"), std::runtime_error);

    try {
        ParseFile("tests/14_exceptions_missing_value.txt");
    }
    catch (std::exception& e) {
        CHECK(std::string(e.what()).substr(18) == ": an exception has been raised.\ntests/14_exceptions_missing_value.txt:0:5: error: expected a value after '='\n\t0 | key =\n\t  |      ^\n\t  |      |\n\t  |      missing value");
    }
}

TEST_CASE("[15_exceptions_missing_operator] operator missing after key") {
    CHECK_THROWS_AS(ParseFile("tests/15_exceptions_missing_operator.txt"), std::runtime_error);

    try {
        ParseFile("tests/15_exceptions_missing_operator.txt");
    }
    catch (std::exception& e) {
        CHECK(std::string(e.what()).substr(18) == ": an exception has been raised.\ntests/15_exceptions_missing_operator.txt:0:3: error: expected an operator after 'key'\n\t0 | key\n\t  |    ^\n\t  |    |\n\t  |    missing operator");
    }
}

TEST_CASE("[16_exceptions_unmatched_closing_bracket] unexpected closing bracket after value") {
    CHECK_THROWS_AS(ParseFile("tests/16_exceptions_unmatched_closing_bracket.txt"), std::runtime_error);

    try {
        ParseFile("tests/16_exceptions_unmatched_closing_bracket.txt");
    }
    catch (std::exception& e) {
        CHECK(std::string(e.what()).substr(18) == ": an exception has been raised.\ntests/16_exceptions_unmatched_closing_bracket.txt:0:12: error: unexpected closing brace '}'\n\t0 | key = value }\n\t  |             ^\n\t  |             |\n\t  |             unmatched closing brace");
    }
}