#include "Jomini.hpp"

namespace Jomini {

//////////////////////////////////////////////////////////
//                  Jomini Objects                      //
//////////////////////////////////////////////////////////

Flags operator|(Flags a, Flags b) {
    return static_cast<Flags>(static_cast<int>(a) | static_cast<int>(b));
}

Flags operator&(Flags a, Flags b) {
    return static_cast<Flags>(static_cast<int>(a) & static_cast<int>(b));
}

Flags operator~(Flags a) {
    return static_cast<Flags>(~static_cast<int>(a));
}

Flags& operator|=(Flags& a, Flags b) {
    return a = a | b;
}

Flags& operator&=(Flags& a, Flags b) {
    return a = a & b;
}

Object::Object()
: m_Value(""), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
{}

Object::Object(int scalar)
: m_Value(std::to_string(scalar)), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
{}

Object::Object(double scalar)
: m_Value(std::to_string(scalar)), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
{}

Object::Object(bool scalar)
: m_Value((scalar ? "yes" : "no")), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
{}

Object::Object(const std::string& scalar)
: m_Value(scalar), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
{}

Object::Object(const Date& scalar)
: m_Value((std::string) scalar), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
{}

Object::Object(const ObjectMap& objects)
: m_Value(objects), m_Type(Type::OBJECT), m_Flags(Flags::NONE)
{}

Object::Object(const ObjectArray& array)
: m_Value(array), m_Type(Type::ARRAY), m_Flags(Flags::NONE)
{}

Object::Object(const std::variant<std::string, ObjectMap, ObjectArray>& value)
: m_Value(value), m_Type((Type) value.index()), m_Flags(Flags::NONE)
{}

Object::Object(const Object& object) {
    std::shared_ptr<Object> copy = object.Copy();
    m_Type = copy->m_Type;
    m_Value = copy->m_Value;
}

Object::Object(const std::shared_ptr<Object>& object)
: Object(*object)
{}

Object::~Object() {}

Type Object::GetType() const {
    return m_Type;
}

bool Object::Is(Type type) const {
    return m_Type == type;
}

std::shared_ptr<Object> Object::Copy() const {
    if (m_Type == Type::SCALAR) {
        return std::make_shared<Object>(m_Value);
    }
    else if (m_Type == Type::OBJECT) {
        // Make a deep copy of each objects in the original map.
        const ObjectMap& originalObjects = std::get<ObjectMap>(m_Value);
        ObjectMap objects;
        for (auto [key, pair] : originalObjects) {
            auto [op, value] = pair;
            objects.insert(key, std::make_pair(op, value->Copy()));
        }
        return std::make_shared<Object>(objects);
    }
    else if (m_Type == Type::ARRAY) {
        // Make a deep copy of each objects in the original array.
        const ObjectArray& originalArray = std::get<ObjectArray>(m_Value);
        ObjectArray array;
        array.reserve(originalArray.size());
        for (auto object : originalArray)
            array.push_back(object->Copy());
        return std::make_shared<Object>(array);
    }
    return nullptr;
}

Flags Object::GetFlags() const {
    return m_Flags;
}

bool Object::HasFlag(Flags flag) const {
    return (bool) (m_Flags & flag);
}

void Object::SetFlags(Flags flags) {
    m_Flags = flags;
}

void Object::SetFlag(Flags flag, bool enabled) {
    if(enabled) m_Flags |= flag;
    else m_Flags &= (~flag);
}

void Object::ConvertToArray() {
    if (m_Type == Type::ARRAY)
        return;
    // If it is currently a scalar, then create an array with it.
    if (m_Type == Type::SCALAR) {
        m_Value = ObjectArray{std::make_shared<Object>(m_Value)};
        m_Type = Type::ARRAY;
    }
    // If it is an object, then turn it into an array with the former object as the only value.
    else if (m_Type == Type::OBJECT) {
        std::shared_ptr<Object> formerObject = std::make_shared<Object>(m_Value);
            
        m_Value = ObjectArray{};
        m_Type = Type::ARRAY;

        // If the former object was not empty, then add it to the array.
        if (!formerObject->GetMap().empty())
            std::get<ObjectArray>(m_Value).push_back(formerObject);
    }
}

void Object::ConvertToObject() {
    if (m_Type == Type::OBJECT)
        return;
    // If it is currently a scalar, then raise an exception.
    if (m_Type == Type::SCALAR) {
        throw std::runtime_error("Invalid conversion of scalar to object.");
    }
    // If it is an empty array, then turn it into an object.
    // Otherwise, raise an exception.
    else if (m_Type == Type::ARRAY) {
        if (!std::get<ObjectArray>(m_Value).empty())
            throw std::runtime_error("Invalid conversion of non-empty array to object.");
        m_Value = ObjectMap{};
        m_Type = Type::OBJECT;
    }
}

template <typename T> T Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to " + std::string(typeid(T).name()));
    try {
        return (T) std::get<std::string>(m_Value);
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to " + std::string(typeid(T).name()));
    }
}

template <> std::string Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to std::string.");
    return std::get<std::string>(m_Value);
}

template <> int Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to int.");
    try {
        return std::stoi(std::get<std::string>(m_Value));
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to int.");
    }
}

template <> double Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to double.");
    try {
        return std::stod(std::get<std::string>(m_Value));
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to double.");
    }
}

template <> bool Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to boolean.");
    if (std::get<std::string>(m_Value) == "yes")
        return true;
    else if (std::get<std::string>(m_Value) == "no")
        return false;
    throw std::runtime_error("Invalid conversion of object to boolean.");
}

template <> Date Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to date.");
    try {
        return Date(std::get<std::string>(m_Value));
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to date.");
    }
}

template <typename T> std::optional<T> Object::AsOpt() const {
    try {
        T value = this->As<T>();
        return value;
    }
    catch (std::exception& e) {}
    return std::nullopt;
}

template <typename T> T Object::As(const T& defaultValue) const {
    try {
        T value = this->As<T>();
        return value;
    }
    catch (std::exception& e) {}
    return defaultValue;
}

template <typename T> std::vector<T> Object::AsArray() const {
    if (m_Type != Type::ARRAY)
        throw std::runtime_error("Invalid conversion of object to array of " + std::string(typeid(T).name()));
    ObjectArray array = std::get<ObjectArray>(m_Value);
    std::vector<T> newArray;
    newArray.reserve(array.size());
    try {
        for (auto obj : array)
            newArray.push_back(obj->As<T>());
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to array of " + std::string(typeid(T).name()));
    }
    return newArray;
}

template std::vector<std::string> Object::AsArray() const;
template std::vector<int> Object::AsArray() const;
template std::vector<double> Object::AsArray() const;
template std::vector<bool> Object::AsArray() const;
template std::vector<Date> Object::AsArray() const;

template <> std::vector<std::shared_ptr<Object>> Object::AsArray() const {
    if (m_Type != Type::ARRAY)
        throw std::runtime_error("Invalid conversion of object to array of object");
    return this->Copy()->GetArray();
}

template <typename T> std::optional<std::vector<T>> Object::AsArrayOpt() const {
    try {
        T value = this->AsArray<T>();
        return value;
    }
    catch (std::exception& e) {}
    return std::nullopt;
}

template <typename T> std::vector<T> Object::AsArray(const T& defaultValue) const {
    try {
        T value = this->AsArray<T>();
        return value;
    }
    catch (std::exception& e) {}
    return defaultValue;
}

bool Object::Contains(const std::string& key) const {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Contains on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Contains on array.");
    return std::get<ObjectMap>(m_Value).contains(key);
}

std::shared_ptr<Object> Object::Get(const std::string& key) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Get on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Get on array.");
    return std::get<ObjectMap>(m_Value).at(key).second;
}

Operator Object::GetOperator(const std::string& key) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use GetOperator on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use GetOperator on array.");
    return std::get<ObjectMap>(m_Value).at(key).first;
}

template <typename T> void Object::Push(T value, bool convertToArray) {
    if (m_Type != Type::ARRAY) {
        if (!convertToArray)
            throw std::runtime_error("Cannot use Push on scalar or object.");
        this->ConvertToArray();
    }
    std::get<ObjectArray>(m_Value).push_back(std::make_shared<Object>(value));
}

template <> void Object::Push(std::shared_ptr<Object> value, bool convertToArray) {
    if (m_Type != Type::ARRAY) {
        if (!convertToArray)
            throw std::runtime_error("Cannot use Push on scalar or object.");
        this->ConvertToArray();
    }
    std::get<ObjectArray>(m_Value).push_back(value);
}

template <typename T> void Object::Put(std::string key, T value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Put on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Put on array.");
    std::get<ObjectMap>(m_Value).insert(key, std::make_pair(op, std::make_shared<Object>(value)));
}

template <> void Object::Put(std::string key, std::shared_ptr<Object> value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Put on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Put on array.");
    std::get<ObjectMap>(m_Value).insert(key, std::make_pair(op, value));
}

template <typename T> void Object::Merge(std::string key, T value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Merge on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Merge on array.");
    ObjectMap& map = std::get<ObjectMap>(m_Value);
    if (map.contains(key)) {
        map.at(key).second->Push(std::make_shared<Object>(value), true);
    }
    else {
        map.insert(key, std::make_pair(op, std::make_shared<Object>(value)));
    }
}

template <> void Object::Merge(std::string key, std::shared_ptr<Object> value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Merge on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Merge on array.");
    ObjectMap& map = std::get<ObjectMap>(m_Value);
    if (map.contains(key)) {
        if (map.at(key).second->HasFlag(Flags::LIST | Flags::RANGE) && value->Is(Type::ARRAY)) {
            for (auto v : value->GetArray())
                map.at(key).second->Push(v, false);
        }
        else {
            map.at(key).second->Push(value, true);
        }
    }
    else {
        map.insert(key, std::make_pair(op, value));
    }
}

std::string& Object::GetString() {
    if (m_Type == Type::OBJECT)
        throw std::runtime_error("Cannot use GetString on object.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use GetString on array.");
    return std::get<std::string>(m_Value);
}

ObjectMap& Object::GetMap() {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use GetMap on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use GetMap on array.");
    return std::get<ObjectMap>(m_Value);
}

ObjectArray& Object::GetArray() {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use GetArray on scalar.");
    if (m_Type == Type::OBJECT)
        throw std::runtime_error("Cannot use GetArray on object.");
    return std::get<ObjectArray>(m_Value);
}

//////////////////////////////////////////////////////////
//                      Parser                          //
//////////////////////////////////////////////////////////

Parser::Parser()
: m_FilePath(""), m_CurrentLine(0), m_CurrentCursor(0), m_PreviousLine(0), m_PreviousCursor(0), m_LastBraceLine(0)
{}

void Parser::ThrowError(const std::string& error, const std::string& cursorError, int cursorOffset, std::string sourceFile, int sourceFileLine) {
    std::vector<std::string> lines = this->GetFileLines();

    std::string message = std::format(
        "{}:{}: an exception has been raised.\n",
        sourceFile,
        sourceFileLine
    );

    message += std::format(
        "{}:{}:{}: error: {}\n",
        m_FilePath,
        m_PreviousLine+1,
        std::max(1, m_PreviousCursor+cursorOffset+1),
        error
    );

    std::string tab1 = std::string(std::to_string(m_PreviousLine).length(), ' ');
    std::string tab2 = std::string(std::max(0, m_PreviousCursor+cursorOffset), ' ');

    // Add the last line with an opening brace if relevent to the exception.
    if (m_LastBraceLine != m_PreviousLine) {
        message += std::format(
            "\t{} | {}\n",
            m_LastBraceLine+1, lines.at(m_LastBraceLine)
        );

        if (m_LastBraceLine != m_PreviousLine-1)
            message += std::format("\t{} | ...\n", tab1);
    }
    
    // Add the last relevent line and the error arrow.
    message += std::format(
        "\t{} | {}\n"
        "\t{} | {}^\n"
        "\t{} | {}|\n"
        "\t{} | {}{}",
        m_PreviousLine+1, lines.at(m_PreviousLine),
        tab1, tab2,
        tab1, tab2,
        tab1, tab2, cursorError
    );

    throw std::runtime_error(message);
}

std::vector<std::string> Parser::GetFileLines() const {
    std::vector<std::string> lines;
    std::ifstream file(m_FilePath);
    if(!file.is_open())
        return lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    file.close();
    return lines;
}

std::shared_ptr<Object> Parser::ParseFile(const std::string& filePath) {
    m_FilePath = filePath;
    std::ifstream file(filePath);
    std::shared_ptr<Object> obj = this->Parse(file);
    file.close();
    return obj;
}

std::shared_ptr<Object> Parser::ParseString(const std::string& content) {
    m_FilePath = "";
    std::istringstream ss(content);
    std::shared_ptr<Object> obj = this->Parse(ss);
    return obj;
}

std::shared_ptr<Object> Parser::Parse(std::istream& stream, int depth) {
    // Initialize the line number on the first function call.
    if (depth == 0) {
        m_CurrentLine = 0;
        m_CurrentCursor = 0;
        m_PreviousLine = 0;
        m_PreviousCursor = 0;
    }

    // Initialize the main object, key and operator.
    // Depending on what is read, the object can be an scalar, an object (map) or an array.
    // Key and operator are not used if it isn't parsing a map object.
    std::shared_ptr<Object> mainObject = std::make_shared<Object>(ObjectMap{});
    std::string key = "";
    Operator op = Operator::EQUAL;
    Flags flags = Flags::NONE;

    #define IS_BLANK(ch) (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
    #define IS_OPERATOR(ch) (ch == '=' || ch == '<' || ch == '>' || ch == '!' || ch == '?')
    #define IS_BRACE(ch) (ch == '{' || ch == '}')
    #define IS_COMMENT(ch) (ch == '#')

    // Read characters from the stream until the end of the string
    // and capture characters into a buffer.
    const auto CaptureTillBlank = [&](bool isStringLiteral) {
        bool isEscaped = false;
        std::string buffer = "";

        while (stream.peek() != std::istream::traits_type::eof()) {
            // If is a string literal:
            // - Keep track of escaped characters.
            // - Stop capture on unescaped quotes (included).
            if (isStringLiteral) {
                int ch = stream.get();
                buffer += ch;
                m_CurrentCursor++;
                if (ch == '\\')
                    isEscaped = true;
                if (!isEscaped && ch == '"')
                    break;
                if (ch != '\\')
                    isEscaped = false;
            }
            // Otherwise:
            // - Keep capturing until first blank character
            //   comment, operator or braces (all excluded).
            else if (IS_BLANK(stream.peek()) || IS_OPERATOR(stream.peek()) || IS_BRACE(stream.peek()) || IS_COMMENT(stream.peek())) {
                break;
            }
            else {
                buffer += stream.get();
                m_CurrentCursor++;
            }
        }
        return buffer;
    };

    // Read and discard characters from the stream until a specific character (excluded).
    const auto SkipTill = [&](char end) {
        while (stream.peek() != std::istream::traits_type::eof()) {
            if (stream.peek() == end)
                return;
            if (stream.peek() == '\n') {
                m_CurrentLine++;
                m_CurrentCursor = 0;
            }
            stream.get();
        }
    };

    // Initialize the current parsing state:
    int state = 1;

    // Loop over one character at a time, until the stream is empty.
    while (stream.peek() != std::istream::traits_type::eof()) {
        int ch = stream.get();
        m_CurrentCursor++;

        // Increment the line number for debugging.
        if (ch == '\n') {
            m_CurrentLine++;
            m_CurrentCursor = 0;
        }

        if (IS_BLANK(ch))
            continue;
        if (IS_COMMENT(ch)) {
            SkipTill('\n');
            continue;
        }

        m_PreviousLine = m_CurrentLine;
        m_PreviousCursor = m_CurrentCursor;

        // State #1a: stop reading return the current main object.
        //  - from: initial, state #3
        //  - next: terminal
        //  - accepts: }
        if (state == 1 && ch == '}') {
            if (depth == 0)
                THROW_ERROR("unexpected closing brace '}'", "unmatched closing brace", -1);
            return mainObject;
        }
        // State #1b: parsing object in array.
        //  - from: initial, state #3
        //  - next: state #4
        //  - accepts: {
        else if (state == 1 && ch == '{') {
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMap().empty())
                THROW_ERROR("unexpected opening brace '{' inside key-value block", "stray opening brace", -1);
            int lastBrace = m_CurrentLine;
            m_LastBraceLine = m_CurrentLine;
            std::shared_ptr<Object> object = this->Parse(stream, depth+1);
            m_LastBraceLine = lastBrace;
            mainObject->Push(object, true);
            key = "";
            state = 4;
        }
        // State #1c: parsing key.
        //  - from: initial, state #3
        //  - next: state #2
        //  - accepts: non-blank, non-operator
        else if (state == 1) {
            if (IS_OPERATOR(ch))
                THROW_ERROR(std::format("expected key before '{}'", OperatorsLabels.at(op)), "missing key", 0);
            key = std::string(1, ch) + CaptureTillBlank(ch == '"');
            state = 2;
        }
        // State #2a: parsing operator after #1.
        //  - from: state #1c
        //  - next: state #3
        //  - accepts: =, <, >, !, ?
        else if (state == 2 && IS_OPERATOR(ch)) {
            if (ch == '!' && stream.peek() != '=')
                THROW_ERROR("unexpected token '!'", "unexpected exclamation mark; did you mean '!='?", -1);
            if (ch == '?' && stream.peek() != '=')
                THROW_ERROR("unexpected token '?'", "unexpected question mark; did you mean '?='?", -1);
            switch (ch) {
                case '=':
                    op = Operator::EQUAL;
                    break;
                case '<':
                    if (stream.peek() == '=') {
                        stream.get();
                        m_CurrentCursor++;
                        op = Operator::LESS_EQUAL;
                        break;
                    }
                    op = Operator::LESS;
                    break;
                case '>':
                    if (stream.peek() == '=') {
                        stream.get();
                        m_CurrentCursor++;
                        op = Operator::GREATER_EQUAL;
                        break;
                    }
                    op = Operator::GREATER;
                    break;
                case '!':
                    stream.get();
                    m_CurrentCursor++;
                    op = Operator::NOT_EQUAL;
                    break;
                case '?':
                    stream.get();
                    m_CurrentCursor++;
                    op = Operator::NOT_NULL;
                    break;
            }
            state = 3;
        }
        // State #2b: parsing an object after a scalar and create an array.
        //  - from: state #1c
        //  - next: state #4
        //  - accepts: {
        else if (state == 2 && ch == '{') {
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMap().empty())
                THROW_ERROR("unexpected opening brace '{' inside key-value block; expected operator", "stray opening brace; did you mean '='?", -1);
            int lastBrace = m_CurrentLine;
            m_LastBraceLine = m_CurrentLine;
            std::shared_ptr<Object> object = this->Parse(stream, depth+1);
            m_LastBraceLine = lastBrace;
            mainObject->Push(key, true);
            mainObject->Push(object);
            key = "";
            state = 4;
        }
        // State #2c: stop parsing a single value array.
        //  - from: state #1c
        //  - next: terminal
        //  - accepts: }
        else if (state == 2 && ch == '}') {
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMap().empty())
                THROW_ERROR("unexpected closing brace '}'; expected '=' or another operator", "unexpected closing brace; did you mean '='?", -1);
            mainObject->Push(key, true);
            return mainObject;
        }
        // State #2d: parsing an array.
        //  - from: state #1c
        //  - next: state #4
        //  - accepts: non-blank
        else if (state == 2) {
            if (IS_BRACE(ch))
                throw std::runtime_error("Failed to parse brace in state #2d");
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMap().empty())
                throw std::runtime_error("Failed to parse object in state #2d");
            std::string buffer = std::string(1, ch) + CaptureTillBlank(ch == '"');
            mainObject->Push(key, true);
            mainObject->Push(buffer);
            key = "";
            state = 4;
        }
        // State #3a: parsing object value.
        //  - from: state #2a, state #3b
        //  - next: state #1
        //  - accepts: {
        else if (state == 3 && ch == '{') {
            int lastBrace = m_CurrentLine;
            m_LastBraceLine = m_CurrentLine;
            std::shared_ptr<Object> object = this->Parse(stream, depth+1);
            m_LastBraceLine = lastBrace;

            // Empty object are by default all map objects, so if there is
            // a list flags attached, the convert it to an array.
            if (object->Is(Type::OBJECT) && ((bool) (flags & (Flags::LIST | Flags::RANGE))))
                object->ConvertToArray();

            // Convert range to an array.
            if ((bool) (flags & Flags::RANGE)) {
                if (!object->Is(Type::ARRAY))
                    throw std::runtime_error("Failed to parse range in state #3a");
                ObjectArray& array = object->GetArray();
                if (array.size() != 2 || !array.at(0)->Is(Type::SCALAR) || !array.at(1)->Is(Type::SCALAR))
                    throw std::runtime_error("Failed to parse range in state #3a");
                int a = array.at(0)->As<int>();
                int b = array.at(1)->As<int>();
                array.clear();
                if (a <= b) for (int i = a; i <= b; i++)
                    array.push_back(std::make_shared<Object>(i));
                else for (int i = a; i >= b; i--)
                    array.push_back(std::make_shared<Object>(i));
            }
            
            mainObject->Merge(key, object, op);
            mainObject->Get(key)->SetFlag(flags, true);
            flags = Flags::NONE;
            key = "";
            state = 1;
        }
        // State #3b: parsing scalar value.
        //  - from: state #2a, state #3b
        //  - next: state #1, state #3
        //  - accepts: non-blank, non-operator
        else if (state == 3) {
            if (IS_OPERATOR(ch))
                throw std::runtime_error("Failed to parse operator in state #3b");
            if (IS_BRACE(ch))
                throw std::runtime_error("Failed to parse brace in state #3b");
            std::string buffer = std::string(1, ch) + CaptureTillBlank(ch == '"');
            // Check if the value correspond to an array flag.
            if (buffer == "rgb")
                flags = Flags::RGB;
            else if (buffer == "hsv")
                flags = Flags::HSV;
            else if (buffer == "LIST")
                flags = Flags::LIST;
            else if (buffer == "RANGE")
                flags = Flags::RANGE;
            else {
                mainObject->Merge(key, std::make_shared<Object>(buffer), op);
                key = "";
                state = 1;
                continue;
            }
            state = 3;
        }
        // State #4a: stop parsing an array.
        //  - from: state #2b, state #2d
        //  - next: terminal
        //  - accepts: }
        else if (state == 4 && ch == '}') {
            if (depth == 0)
                throw std::runtime_error("Failed to parse brace in state #4a");
            return mainObject;
        }
        // State #4b: start parsing an object inside an array.
        //  - from: state #2b, state #2d
        //  - next: state #4
        //  - accepts: {
        else if (state == 4 && ch == '{') {
            int lastBrace = m_CurrentLine;
            m_LastBraceLine = m_CurrentLine;
            std::shared_ptr<Object> object = this->Parse(stream, depth+1);
            m_LastBraceLine = lastBrace;
            mainObject->Push(object);
            key = "";
            state = 4;
        }
        // State #4c: continue parsing an array.
        //  - from: state #2b, state #2d
        //  - next: state #4
        //  - accepts: non-blank
        else if (state == 4) {
            if (IS_OPERATOR(ch))
                throw std::runtime_error("Failed to parse operator in state #4c");
            if (IS_BRACE(ch))
                throw std::runtime_error("Failed to parse brace in state #4c");
            std::string buffer = std::string(1, ch) + CaptureTillBlank(ch == '"');
            mainObject->Push(buffer);
            state = 4;
        }

        // Update the previous line and cursor number to take into account
        // strings and operators that have been read in the states.
        m_PreviousLine = m_CurrentLine;
        m_PreviousCursor = m_CurrentCursor;
    }

    if (depth == 0 && mainObject->Is(Type::SCALAR))
        THROW_ERROR("unexpected value at root level", "unexpected standalone value", -INT_MAX);
    if (depth == 0 && mainObject->Is(Type::ARRAY))
        THROW_ERROR("unexpected array at root level", "unexpected standalone value", -INT_MAX);

    if (!key.empty() && state == 3)
        THROW_ERROR(std::format("expected a value after '{}'", OperatorsLabels.at(op)), "missing value", 0);
    if (!key.empty() && state == 2)
        THROW_ERROR(std::format("expected an operator after '{}'", key), "missing operator", 0);
    if (state == 4)
        THROW_ERROR("expected closing brace '}'", "unmatched closing brace", 1);
    if (depth > 0 && mainObject->Is(Type::OBJECT))
        THROW_ERROR("expected closing brace '}'", "unmatched closing brace", 1);

    return mainObject;
}

std::shared_ptr<Object> ParseFile(const std::string& filePath) {
    Parser parser;
    return parser.ParseFile(filePath);
}

std::shared_ptr<Object> ParseString(const std::string& content) {
    Parser parser;
    return parser.ParseString(content);
}

std::shared_ptr<Object> Parse(std::istream& stream) {
    Parser parser;
    return parser.Parse(stream);
}

}