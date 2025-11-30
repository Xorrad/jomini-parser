# jomini-parser - <span style="font-size: 16px; vertical-align: middle;">a script parser for Crusader Kings 3.</span>

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![GitHub issues](https://img.shields.io/github/issues/xorrad/jomini-parser)](https://github.com/xorrad/jomini-parser/issues)
[![GitHub stars](https://img.shields.io/github/stars/xorrad/jomini-parser.svg?style=flat&label=stars)](https://github.com/xorrad/jomini-parser)

Lightweight header-only C++ parser and in-memory representation for Paradox-style key/array/object/operator scripting files with rich error reporting.

## Table of contents

- [Quick overview](#quick-overview)
- [Features](#features)
- [Build & run](#build--run)
- [How to use](#how-to-use)
- [Examples](#examples)
- [Contributing](#contributing)
- [Third-parties](#third-parties)
- [License](#license)

---

# Quick overview

Jomini parses both text and files written in Paradox Interactive’s scripting syntax (key–value pairs, nested {} blocks, arrays, operators, colors, dates, booleans, numbers, and strings). While it can also handle scripts from games like EU4, CK2, or HOI4, the parser is primarily designed and optimized for Crusader Kings 3.

It exposes a single type, **`Object`**, which can be:

- an **object/map** (`key → (operator, Object)`),
- an **array** (ordered),
- a **scalar** (stored internally as a string and converted only on demand via `As<T>()`).

This parser intentionally keeps **all scalars as raw strings**, delaying conversion until you explicitly call:

```cpp
obj->As<T>();       // convert scalar to T at runtime
obj->As<T>(def);    // convert or fallback to default
```

This design makes the parser highly forgiving and flexible, allowing it to tolerate unexpected or irregular syntax. As a result, many issues that would normally cause parsing to stop are instead deferred to the data-handling stage, preventing premature failure when reading the file.

---

# Features

- Parse files into a simple `Object` tree with `ParseFile(path)`.
- Scalar-to-type conversion **on demand** using `Object::As<T>()`.
- Arrays convertible to `std::vector<T>` using `AsArray<T>()`.
- Operators supported: `=`, `<`, `<=`, `>`, `>=`, `!=`, `?=`.
- Arrays support flags (`RGB`, `HSV`, `LIST`, `RANGE`).
- Keys preserve insertion order.
- Rich error handling with file/line/column diagnostics.
- UTF-8 support for keys & values.
- Includes a built-in adapter for `sf::Color`. This exists because the project was originally developed for my CK3 map editor, meckt, which relies on SFML and therefore uses the `sf::Color` data structure.

---

# Build & run

### Make

```bash
make
```

### g++ example (simple)

```bash
g++-13 -std=c++20 src/*.cpp -DNSFML -I. -Ivendor -L/usr/lib -lbfd -ldw -o main
./main
```

### Adding to your project

To use Jomini in your project, simply include the Jomini.hpp header and add Jomini.cpp to your build. No additional installation or linking is required—these two files contain the full parser implementation, so including them in your project is all that’s needed to start using the library.

---

# How to use

## Parse a file

```cpp
auto root = Jomini::ParseFile("config.txt");
```

Returns a `std::shared_ptr<Jomini::Object>` whose type is `OBJECT`.

Throws `std::runtime_error` with detailed diagnostics on malformed input such as below:

```
tests/00_tests.txt:6:16: error: unexpected opening brace '{' inside key-value block  
	2 | key = {  
	  | ...  
	6 | eazeae = eaze{  
	  |              ^  
	  |              |  
	  |              stray opening brace  
```

---

## Inspecting and navigating objects

```cpp
if (root->Contains("settings")) {
    auto s = root->Get("settings");   // returns std::shared_ptr<Jomini::Object>
    if (s->Is(Jomini::Type::OBJECT)) {
        auto val = s->Get("fullscreen");
    }
}
```

### Check type

```cpp
if (val->Is(Jomini::Type::SCALAR)) { ... }
```

### Get operator for a key

```cpp
auto op = s->GetOperator("fullscreen"); // returns Jomini::Operator
```

---

## Converting values

**All scalar values are stored as strings.**  
They are only converted when you request a type:

```cpp
bool fullscreen = s->Get("fullscreen")->As<bool>();
int width       = s->Get("width")->As<int>();
double scale    = s->Get("scale")->As<double>();
std::string name = s->Get("title")->As<std::string>();
```

With default fallback:

```cpp
int threads = root->Get("threads")->As<int>(4);    // returns 4 if missing or invalid
```

---

## Arrays

```cpp
auto arr     = root->Get("tags");
auto strings = arr->AsArray<std::string>();   // returns vector<std::string>
```

For color arrays:

```cpp
sf::Color c = root->Get("primary_color")->As<sf::Color>();
```

An adapter for `sf::Color` is already implemented.

---

## Serialization

```cpp
std::cout << root->Serialize() << "\n";
```

---

# Examples

## Iterating entries

```cpp
auto root = ParseFile("game_rules.txt");

auto rules = root->Get("rules");
for (auto& [key, pair] : rules->GetMap()) {
    auto& [op, obj] = pair;
    std::cout << key << " = " << obj->As<std::string>() << "\n";
}
```

## Creating objects programmatically

```cpp
auto o = std::make_shared<Jomini::Object>();
o->Put("difficulty", std::make_shared<Jomini::Object>("hard"), Jomini::Operator::EQUAL);
```

## Working with arrays

```cpp
auto arrObj = std::make_shared<Jomini::Object>(
    std::vector<std::string>{"easy", "normal", "hard"}
);

auto v = arrObj->AsArray<std::string>();
```

---

# Contributing

Contributions to the project are highly appreciated! There are several ways to get involved: you can contribute by reporting any issues you encounter, suggesting new features that could enhance the project, or even by actively participating in the development process through the submission of pull requests.

# Third-party components

- **Doctest** (https://github.com/doctest/doctest) : used only by the test suite.
- **Backward-cpp** (https://github.com/bombela/backward-cpp) : used only for nicer stack traces during development (not required to use the library in your own project).

# License

This project is licensed under the MIT License - see the [LICENSE](https://raw.githubusercontent.com/Xorrad/ck3-meckt/master/LICENSE) file for details.