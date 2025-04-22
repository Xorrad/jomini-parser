#include "Jomini.hpp"

namespace Jomini {

//////////////////////////////////////////////////////////
//                         Date                         //
//////////////////////////////////////////////////////////

Date::Date() : year(0), month(0), day(0) {}
Date::Date(int y, int m, int d) : year(y), month(m), day(d) {}

Date::Date(const std::string& str) {
    size_t dot1 = str.find('.');
    if (dot1 == std::string::npos || dot1 == 0)
        throw std::invalid_argument("Date::constructor: invalid date format.");

    size_t dot2 = str.find('.', dot1 + 1);
    if (dot2 == std::string::npos || dot2 == dot1 + 1 || dot2 == str.length() - 1)
        throw std::invalid_argument("Date::constructor: invalid date format.");

    try {
        year = std::stoi(str.substr(0, dot1));
    }
    catch (std::exception& e) {
        throw std::invalid_argument("Date::constructor: invalid year number format.");
    }

    try {
        month = std::stoi(str.substr(dot1 + 1, dot2 - dot1));
    }
    catch (std::exception& e) {
        throw std::invalid_argument("Date::constructor: invalid month number format.");
    }
    
    try {
        day = std::stoi(str.substr(dot2 + 1, str.length() - dot2));
    }
    catch (std::exception& e) {
        throw std::invalid_argument("Date::constructor: invalid day number format.");
    }
}

Date::operator std::string() const {
    return std::to_string(year) + "." + std::to_string(month) + "." + std::to_string(day);
}

bool Date::operator ==(const Date& other) const {
    return year == other.year && month == other.month && day == other.day;
}

bool Date::operator <=(const Date& other) const {
    return !(*this > other);
}

bool Date::operator >=(const Date& other) const {
    return !(*this < other);
}

bool Date::operator <(const Date& other) const {
    return year < other.year
        || (year == other.year && month < other.month)
        || (year == other.year && month == other.month && day < other.day);
}

bool Date::operator >(const Date& other) const {
    return year > other.year
        || (year == other.year && month > other.month)
        || (year == other.year && month == other.month && day > other.day);
}

//////////////////////////////////////////////////////////
//                 Ordered Object Map                   //
//////////////////////////////////////////////////////////

const ObjectMap::Value ObjectMap::s_DefaultValue = { Operator::EQUAL, nullptr };

ObjectMap::ObjectMap() {}

ObjectMap::ObjectMap(const std::unordered_map<std::string, Value>& entries) {
    // m_Items.resize(entries.size());
    for (const auto& [key, value] : entries)
        this->insert(key, value);
}

void ObjectMap::insert(const std::string& key, const Value& value) {
    auto it = m_Index.find(key);
    if (it == m_Index.end()) {
        m_Items.emplace_back(key, value);
        m_Index.emplace(key, --m_Items.end());
    } else {
        it->second->second = value;
    }
}

void ObjectMap::insert(std::string_view key, const Value& value) {
    auto it = m_Index.find(key);
    if (it == m_Index.end()) {
        std::string str(key);
        m_Items.emplace_back(str, value);
        m_Index.emplace(str, --m_Items.end());
    } else {
        it->second->second = value;
    }
}

void ObjectMap::insert_missing(std::string_view key, const Value& value) {
    std::string str(key);    
    m_Items.emplace_back(str, value);
    m_Index.emplace(str, --m_Items.end());
}

template <typename K> void ObjectMap::erase(const K& key) {
    auto it = m_Index.find(key);
    if (it != m_Index.end()) {
        m_Items.erase(it->second);
        m_Index.erase(it);
    }
}

void ObjectMap::clear() {
    m_Items.clear();
    m_Index.clear();
}

template <typename K> ObjectMap::Value& ObjectMap::at(const K& key) {
    auto it = m_Index.find(key);
    if (it == m_Index.end())
        throw std::out_of_range("ObjectMap::at: key not found");
    return it->second->second;
}

template <typename K> const ObjectMap::Value& ObjectMap::at(const K& key) const {
    auto it = m_Index.find(key);
    if (it == m_Index.end())
        throw std::out_of_range("ObjectMap::at: key not found");
    return it->second->second;
}

ObjectMap::Value& ObjectMap::operator[](std::string_view key) {
    auto it = m_Index.find(key);
    if (it == m_Index.end()) {
        std::string str = std::string(key);
        m_Items.emplace_back(str, s_DefaultValue);
        m_Index.emplace(str, --m_Items.end());
        return m_Items.back().second;
    }
    return it->second->second;
}

template <typename K> const ObjectMap::Value& ObjectMap::operator[](const K& key) const {
    auto it = m_Index.find(key);
    return (it != m_Index.end()) ? it->second->second : s_DefaultValue;
}

template <typename K> ObjectMap::Iterator ObjectMap::find(const K& key) {
    auto it = m_Index.find(key);
    return (it != m_Index.end()) ? it->second : m_Items.end();
}

template <typename K> ObjectMap::ConstIterator ObjectMap::find(const K& key) const {
    auto it = m_Index.find(key);
    return (it != m_Index.end()) ? it->second : m_Items.cend();
}

template <typename K> bool ObjectMap::contains(const K& key) const {
    return m_Index.contains(key);
}

ObjectMap::Iterator ObjectMap::begin() {
    return m_Items.begin();
}

ObjectMap::Iterator ObjectMap::end() {
    return m_Items.end();
}

ObjectMap::ConstIterator ObjectMap::begin() const {
    return m_Items.cbegin();
}

ObjectMap::ConstIterator ObjectMap::end() const {
    return m_Items.cend();
}

std::vector<std::string_view> ObjectMap::keys() const {
    std::vector<std::string_view> keys;
    for (const auto& [key, value] : m_Items)
        keys.push_back(std::string_view(key));
    return keys;
}

std::size_t ObjectMap::size() const {
    return m_Items.size();
}

bool ObjectMap::empty() const {
    return m_Items.empty();
}

void ObjectMap::reserve(size_t size) {
    m_Index.reserve(size);
}

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
: m_Value(ObjectMap{}), m_Type(Type::OBJECT), m_Flags(Flags::NONE)
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

Object::Object(const std::string_view& view)
: m_Value(std::string(view)), m_Type(Type::SCALAR), m_Flags(Flags::NONE)
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
    if (enabled) m_Flags |= flag;
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
        if (!formerObject->GetMapUnsafe().empty())
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

template <typename T> std::vector<T> Object::AsArray(const std::vector<T>& defaultValue) const {
    try {
        T value = this->AsArray<T>();
        return value;
    }
    catch (std::exception& e) {}
    return defaultValue;
}

bool Object::Contains(std::string_view key) const {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Contains on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Contains on array.");
    return std::get<ObjectMap>(m_Value).contains(key);
}

std::shared_ptr<Object> Object::Get(std::string_view key) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Get on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Get on array.");
    return std::get<ObjectMap>(m_Value).at(key).second;
}

Operator Object::GetOperator(std::string_view key) {
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

void Object::Remove(std::string_view key) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Remove on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Remove on array.");
    std::get<ObjectMap>(m_Value).erase(key);
}

template <typename T> void Object::Put(std::string_view key, T value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Put on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Put on array.");
    std::get<ObjectMap>(m_Value).insert(key, std::make_pair(op, std::make_shared<Object>(value)));
}

template <> void Object::Put(std::string_view key, std::shared_ptr<Object> value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Put on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Put on array.");
    std::get<ObjectMap>(m_Value).insert(key, std::make_pair(op, value));
}

template <typename T> void Object::Merge(std::string_view key, T value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Merge on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Merge on array.");
    ObjectMap& map = std::get<ObjectMap>(m_Value);
    auto it = map.find(key);
    if (it != map.end()) {
        it->second.second->Push(std::make_shared<Object>(value), true);
    }
    else {
        // TODO: improve this to avoid calling map.find again.
        map.insert(key, std::make_pair(op, std::make_shared<Object>(value)));
    }
}

template <> void Object::Merge(std::string_view key, std::shared_ptr<Object> value, Operator op) {
    if (m_Type == Type::SCALAR)
        throw std::runtime_error("Cannot use Merge on scalar.");
    if (m_Type == Type::ARRAY)
        throw std::runtime_error("Cannot use Merge on array.");
    ObjectMap& map = std::get<ObjectMap>(m_Value);
    auto it = map.find(key);
    if (it != map.end()) {
        if (it->second.second->HasFlag(Flags::LIST | Flags::RANGE) && value->Is(Type::ARRAY)) {
            for (auto v : value->GetArray())
                it->second.second->Push(v, false);
        }
        else {
            it->second.second->Push(value, true);
        }
    }
    else {
        // TODO: improve this to avoid calling map.find again.
        map.insert(key, std::make_pair(op, value));
    }
}

template <typename T> void Object::MergeUnsafe(std::string_view key, T value, Operator op) {
    ObjectMap& map = std::get<ObjectMap>(m_Value);
    auto it = map.find(key);

    if (it != map.end()) {
        it->second.second->Push(std::make_shared<Object>(value), true);
    }
    else {
        map.insert_missing(key, ObjectMap::Value(op, std::make_shared<Object>(value)));
    }
}

template <> void Object::MergeUnsafe(std::string_view key, std::shared_ptr<Object> value, Operator op) {
    ObjectMap& map = std::get<ObjectMap>(m_Value);
    auto it = map.find(key);

    if (it != map.end()) {
        if (it->second.second->HasFlag(Flags::LIST | Flags::RANGE) && value->Is(Type::ARRAY)) {
            for (auto v : value->GetArray())
                it->second.second->Push(v, false);
        }
        else {
            it->second.second->Push(value, true);
        }
    }
    else {
        map.insert_missing(key, ObjectMap::Value(op, value));
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

ObjectMap& Object::GetMapUnsafe() {
    return std::get<ObjectMap>(m_Value);
}

ObjectArray& Object::GetArrayUnsafe() {
    return std::get<ObjectArray>(m_Value);
}

std::string Object::Serialize(uint depth, bool isInline) const {
    if (m_Type == Type::OBJECT)
        return this->SerializeObject(depth, isInline);
    if (m_Type == Type::ARRAY)
        return this->SerializeArray(depth);
    return this->SerializeScalar(depth);
}

std::string Object::SerializeScalar(uint depth) const {
    if (m_Type != Type::SCALAR)
        return "";
    return std::get<std::string>(m_Value);
}

std::string Object::SerializeObject(uint depth, bool isInline) const {
    if (m_Type != Type::OBJECT)
        return "";  
    const ObjectMap& map = std::get<ObjectMap>(m_Value);

    // An empty object is an empty-string on first depth, { } otherwise.
    if (map.empty())
        return (depth > 0) ? "{ }" : "";

    // Format recursively objects in the current object.
    std::string lines = "";

    for (auto it = map.begin(); it != map.end(); it++) {
        if (it != map.begin())
            lines.append(isInline ? " " : "\n");

        if (it->second.second->HasFlag(Flags::LIST) || it->second.second->HasFlag(Flags::RANGE)) {
            lines.append(
                it->second.second->SerializeArrayRange(it->first, it->second.first, depth)
            );
            continue;
        }

        lines.append(std::format(
            "{}{} {} {}",
            std::string((isInline ? 0 : depth), '\t'), // Indentation
            it->first, // Key
            OperatorsLabels.at(it->second.first), // Operator
            it->second.second->Serialize(depth+1, isInline) // Value
        ));
    }

    // On first depth, the object is formatted as ..., instead of {...} 
    if (depth == 0)
        return lines;
    if (isInline)
        return std::format("{{ {} }}", lines);
    return std::format("{{\n{}\n{}}}", lines, std::string(depth-1, '\t'));
}

std::string Object::SerializeArray(uint depth) const {
    if (m_Type != Type::ARRAY)
        return "";
    const ObjectArray& array = std::get<ObjectArray>(m_Value);

    if (array.empty())
        return "{ }";

    std::string lines = "{ ";
    std::string indent = std::string(depth, '\t');
    bool hasObjectOrArray = false;

    for (auto it = array.begin(); it != array.end(); it++) {
        if ((*it)->Is(Type::SCALAR)) {
            if (!lines.empty() && lines.at(lines.size()-1) == '\n')
                lines.append(indent);
            lines.append(std::format("{} ", (*it)->As<std::string>()));
            continue;
        }
        if (!lines.empty() && lines.at(lines.size()-1) != '\n')
            lines.append("\n");
        lines.append(std::format("{}{}{}", indent, (*it)->Serialize(depth+1, true), (it == std::prev(array.end()) ? "" : "\n")));
        hasObjectOrArray = true;
    }

    if (hasObjectOrArray)
        lines.append("\n" + std::string(std::max(0U, depth-1), '\t'));
    lines.append("}");

    return lines;
}

std::string Object::SerializeArrayRange(const std::string& key, Operator op, uint depth) const {
    std::vector<int> l = this->AsArray<int>();
    std::vector<int> loneNumbers;
    std::string indent = std::string(depth, '\t');

    // Make a list of the lines to build the list
    // with LIST and RANGE depending on the values.
    std::string lines = "";
    int count = 0;
    int start = 0;
    int current = 1;

    while (current <= l.size()) {
        // Count the number of elements in the current range and make
        // a range only if there are at least 3 elements.
        int n = current - start;

        // Push a new line if a streak is broken or if it is the last element of the vector.
        if (current == l.size() || l[current-1]+1 != l[current]) {
            if (n > 3) {
                lines.append(std::format("{}{}{} {} RANGE {{ {} {} }}", (count == 0 ? "" : "\n"), indent, key, OperatorsLabels.at(op), l[start], l[current-1]));
                count++;
            }
            else {
                for(int i = start; i < current; i++)
                    loneNumbers.push_back(l[i]);
            }
            start = current;
        }
        current++;
    }

    if (!loneNumbers.empty()) {
        if (count > 0)
            lines.append("\n");
        lines.append(std::format("{}{} {} LIST {{ ", indent, key, OperatorsLabels.at(op)));
        for (auto it = loneNumbers.begin(); it != loneNumbers.end(); it++)
            lines.append(std::format("{}{}", (it == loneNumbers.begin() ? "" : " "), *it));
        lines.append(" }");
    }

    return lines;
}

//////////////////////////////////////////////////////////
//                      Reader                          //
//////////////////////////////////////////////////////////

Reader::Reader() {}

Reader::Reader(std::string filePath) : Reader() {}

Reader::~Reader() {}

void Reader::OpenFile(std::string filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        return;
    this->Open(file);
    file.close();
}

void Reader::OpenString(std::string content) {
    std::istringstream stream(content);
    this->Open(stream);
}

void Reader::Open(std::istream& stream) {
    // Initialize member variables.
    m_Buffer.clear();
    m_View = std::string_view{};
    m_CurrentLine = 0;
    m_CurrentCursor = 0;
    m_CurrentGlobalCursor = 0;

    // Copy the whole file into the buffer.
    stream.seekg(0, std::ios::end);
    std::streamsize size = stream.tellg();
    stream.seekg(0, std::ios::beg);
    m_Buffer.resize(static_cast<size_t>(size));
    stream.read(m_Buffer.data(), size);

    // Initialize the string view using the buffer.
    m_View = std::string_view(m_Buffer);
}

bool Reader::IsEmpty() {
    return m_CurrentGlobalCursor >= m_View.size();
}

char Reader::Read() {
    if (m_CurrentGlobalCursor >= m_View.size())
        throw std::out_of_range("tried to read character outside of buffer bounds.");
    this->IncrementLine();
    return m_View[m_CurrentGlobalCursor++];
}

char Reader::Peek() {
    if (m_CurrentGlobalCursor >= m_View.size())
        return '\0';
    this->IncrementLine();
    return m_View[m_CurrentGlobalCursor++];
}

bool Reader::Match(char expected) {
    if (m_CurrentGlobalCursor < m_View.size() && m_View[m_CurrentGlobalCursor] == expected) {
        this->IncrementLine();
        m_CurrentGlobalCursor++;
        return true;
    }
    return false;
}

std::string_view Reader::ReadUntil(const std::function<bool(char)>& predicate, bool includePrevious, bool includeLast) {
    if (m_CurrentGlobalCursor >= m_View.size())
        return std::string_view{};
    size_t start = m_CurrentGlobalCursor - (size_t) includePrevious;
    size_t end = start + (size_t) includePrevious;
    while (end < m_View.size() && !predicate(m_View[end]))
        end++;
    if (includeLast)
        end++;
    m_CurrentGlobalCursor = end;
    m_CurrentCursor += end-start;
    return m_View.substr(start, end - start);
}

void Reader::SkipUntil(const std::function<bool(char)>& predicate) {
    if (m_CurrentGlobalCursor >= m_View.size())
        return;
    size_t pos = m_CurrentGlobalCursor;
    while (pos < m_View.size() && !predicate(m_View[pos])) {
        pos++;
        m_CurrentCursor++;
    }
    m_CurrentGlobalCursor = pos;
    return;
}

std::string_view Reader::GetView() const {
    return m_View;
}

std::string_view Reader::GetLine(uint line) const {
    // This code does not need to be performant as
    // it is only used if an exception is raised.
    size_t start = 0;
    for (uint n = 0; n < line; n++) {
        size_t pos = m_View.find('\n', start);
        if (pos == std::string_view::npos)
            return {};
        start = pos + 1;
    }
    size_t end = m_View.find('\n', start);
    return m_View.substr(start, (end == std::string_view::npos ? m_View.size() : end) - start);
}

uint Reader::GetCurrentLine() const {
    return m_CurrentLine;
}

uint Reader::GetCurrentCursor() const {
    return m_CurrentCursor;
}

void Reader::IncrementLine() {
    if (m_View[m_CurrentGlobalCursor] == '\n') {
        m_CurrentLine++;
        m_CurrentCursor = 0;
        return;
    }
    if (m_CurrentGlobalCursor < m_View.size()-1)
        m_CurrentCursor++;
}

//////////////////////////////////////////////////////////
//                      Parser                          //
//////////////////////////////////////////////////////////

Parser::Parser()
: m_FilePath(""), m_Reader(Reader()), m_PreviousLine(0), m_PreviousCursor(0), m_LastBraceLine(0)
{}

void Parser::ThrowError(const std::string& error, const std::string& cursorError, int cursorOffset, std::string sourceFile, int sourceFileLine) {
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
            m_LastBraceLine+1, m_Reader.GetLine(m_LastBraceLine)
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
        m_PreviousLine+1, m_Reader.GetLine(m_PreviousLine),
        tab1, tab2,
        tab1, tab2,
        tab1, tab2, cursorError
    );

    throw std::runtime_error(message);
}

std::shared_ptr<Object> Parser::ParseFile(const std::string& filePath) {
    // Initialize the reader with the file.
    m_FilePath = filePath;
    m_Reader.OpenFile(filePath);

    // Initialize the line number on the first function call.
    m_PreviousLine = 0;
    m_PreviousCursor = 0;
    m_LastBraceLine = 0;

    // Parse the file recursively from the root.
    std::shared_ptr<Object> obj = this->Parse(0);
    return obj;
}

std::shared_ptr<Object> Parser::ParseString(const std::string& content) {
    // Initialize the reader with the string.
    m_FilePath = "";
    m_Reader.OpenString(content);

    // Initialize the line number on the first function call.
    m_PreviousLine = 0;
    m_PreviousCursor = 0;
    m_LastBraceLine = 0;

    // Parse the file recursively from the root.
    std::shared_ptr<Object> obj = this->Parse(0);
    return obj;
}

#define IS_BLANK(ch) (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
#define IS_OPERATOR(ch) (ch == '=' || ch == '<' || ch == '>' || ch == '!' || ch == '?')
#define IS_BRACE(ch) (ch == '{' || ch == '}')
#define IS_COMMENT(ch) (ch == '#')

const auto newLinePredicate = [](char c){ return c == '\n'; };
const auto blankPredicate = [](char c){ return IS_BLANK(c) || IS_OPERATOR(c) || IS_BRACE(c) || IS_COMMENT(c); };
const auto quotePredicate = [](char c){ return c == '"'; };

std::shared_ptr<Object> Parser::Parse(int depth) {
    // Initialize the main object, key and operator.
    // Depending on what is read, the object can be an scalar, an object (map) or an array.
    // Key and operator are not used if it isn't parsing a map object.
    std::shared_ptr<Object> mainObject = std::make_shared<Object>(ObjectMap{});
    std::string_view key = "";
    Operator op = Operator::EQUAL;
    Flags flags = Flags::NONE;

    // Initialize the current parsing state:
    int state = 1;

    // Loop over one character at a time, until the stream is empty.
    while (!m_Reader.IsEmpty()) {
        char ch = m_Reader.Read();

        if (IS_BLANK(ch))
            continue;
        if (IS_COMMENT(ch)) {
            m_Reader.SkipUntil(newLinePredicate);
            continue;
        }

        m_PreviousLine = m_Reader.GetCurrentLine();
        m_PreviousCursor = m_Reader.GetCurrentCursor()-1;

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
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMapUnsafe().empty())
                THROW_ERROR("unexpected opening brace '{' inside key-value block", "stray opening brace", 0);
            int lastBrace = m_Reader.GetCurrentLine();
            m_LastBraceLine = lastBrace;
            std::shared_ptr<Object> object = this->Parse(depth+1);
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
            key = m_Reader.ReadUntil((ch == '"' ? quotePredicate : blankPredicate), true, ch == '"');
            state = 2;
        }
        // State #2a: parsing operator after #1.
        //  - from: state #1c
        //  - next: state #3
        //  - accepts: =, <, >, !, ?
        else if (state == 2 && IS_OPERATOR(ch)) {
            if (ch == '!' && !m_Reader.Match('='))
                THROW_ERROR("unexpected token '!'", "unexpected exclamation mark; did you mean '!='?", -1);
            if (ch == '?' && !m_Reader.Match('='))
                THROW_ERROR("unexpected token '?'", "unexpected question mark; did you mean '?='?", -1);
            switch (ch) {
                case '=':
                    op = Operator::EQUAL;
                    break;
                case '<':
                    op = (m_Reader.Match('=') ? Operator::LESS_EQUAL : Operator::LESS);
                    break;
                case '>':
                    op = (m_Reader.Match('=') ? Operator::GREATER_EQUAL : Operator::GREATER);
                    break;
                case '!':
                    op = Operator::NOT_EQUAL;
                    break;
                case '?':
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
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMapUnsafe().empty())
                THROW_ERROR("unexpected opening brace '{' inside key-value block; expected operator", "stray opening brace; did you mean '='?", -1);
            int lastBrace = m_Reader.GetCurrentLine();
            m_LastBraceLine = lastBrace;
            std::shared_ptr<Object> object = this->Parse(depth+1);
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
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMapUnsafe().empty())
                THROW_ERROR("unexpected closing brace '}'; expected '=' or another operator", "unexpected closing brace; did you mean '='?", 0);
            mainObject->Push(key, true);
            return mainObject;
        }
        // State #2d: parsing an array.
        //  - from: state #1c
        //  - next: state #4
        //  - accepts: non-blank
        else if (state == 2) {
            if (mainObject->Is(Type::OBJECT) && !mainObject->GetMapUnsafe().empty())
                THROW_ERROR("unexpected value after key inside key-value block; expected operator", "unexpected value", -1);
            std::string_view buffer = m_Reader.ReadUntil((ch == '"' ? quotePredicate : blankPredicate), true, ch == '"');
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
            int lastBrace = m_Reader.GetCurrentLine();
            m_LastBraceLine = lastBrace;
            std::shared_ptr<Object> object = this->Parse(depth+1);
            m_LastBraceLine = lastBrace;

            // Empty object are by default all map objects, so if there is
            // a list flags attached, the convert it to an array.
            if (object->Is(Type::OBJECT) && ((bool) (flags & (Flags::LIST | Flags::RANGE))))
                object->ConvertToArray();

            // Convert range to an array.
            if ((bool) (flags & Flags::RANGE)) {
                if (!object->Is(Type::ARRAY))
                    THROW_ERROR("expected 2-number-array in RANGE block", "expected array", 0);
                ObjectArray& array = object->GetArray();
                if (array.size() != 2 || !array.at(0)->Is(Type::SCALAR) || !array.at(1)->Is(Type::SCALAR))
                    THROW_ERROR("expected 2-number-array in RANGE block", "expected 2 numbers", 0);
                int a = array.at(0)->As<int>();
                int b = array.at(1)->As<int>();
                array.clear();
                if (a <= b) for (int i = a; i <= b; i++)
                    array.push_back(std::make_shared<Object>(i));
                else for (int i = a; i >= b; i--)
                    array.push_back(std::make_shared<Object>(i));
            }
            mainObject->MergeUnsafe(key, object, op);
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
                THROW_ERROR(std::format("unexpected '{}' after operator inside key-value block", (char) ch), "unexpected operator", 0);
            if (IS_BRACE(ch))
                THROW_ERROR("unexpected closing brace '}' after operator inside key-value block", "unexpected closing brace", 0);
            std::string_view buffer = m_Reader.ReadUntil((ch == '"' ? quotePredicate : blankPredicate), true, ch == '"');
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
                mainObject->MergeUnsafe(key, std::make_shared<Object>(buffer), op);
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
                THROW_ERROR("unexpected closing brace '}'", "unmatched closing brace", 0);
            return mainObject;
        }
        // State #4b: start parsing an object inside an array.
        //  - from: state #2b, state #2d
        //  - next: state #4
        //  - accepts: {
        else if (state == 4 && ch == '{') {
            int lastBrace = m_Reader.GetCurrentLine();
            m_LastBraceLine = lastBrace;
            std::shared_ptr<Object> object = this->Parse(depth+1);
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
                THROW_ERROR(std::format("unexpected '{}' inside array block", (char) ch), "unexpected operator", 0);
            std::string_view buffer = m_Reader.ReadUntil((ch == '"' ? quotePredicate : blankPredicate), true, ch == '"');
            mainObject->Push(buffer);
            state = 4;
        }

        // Update the previous line and cursor number to take into account
        // strings and operators that have been read in the states.
        m_PreviousLine = m_Reader.GetCurrentLine();
        m_PreviousCursor = m_Reader.GetCurrentCursor();
    }

    if (depth == 0 && mainObject->Is(Type::SCALAR))
        THROW_ERROR("unexpected value at root level", "unexpected standalone value", -INT_MAX);
    if (depth == 0 && mainObject->Is(Type::ARRAY))
        THROW_ERROR("unexpected array at root level", "unexpected standalone value", -INT_MAX);

    if (!key.empty() && state == 3)
        THROW_ERROR(std::format("expected a value after '{}'", OperatorsLabels.at(op)), "missing value", 0);
    if (!key.empty() && state == 2)
        THROW_ERROR(std::format("expected an operator after '{}'", key), "missing operator", -1);
    if (state == 4)
        THROW_ERROR("expected closing brace '}'", "unmatched closing brace", -1);
    if (depth > 0 && mainObject->Is(Type::OBJECT))
        THROW_ERROR("expected closing brace '}'", "unmatched closing brace", 2);

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

}