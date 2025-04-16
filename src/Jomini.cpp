#include "Jomini.hpp"

using namespace Jomini;

Object::Object() {}

Object::Object(std::string scalar)
: m_Scalar(scalar), m_Type(Type::SCALAR)
{}

Object::Object(std::map<std::string, std::shared_ptr<Object>> objects)
: m_Objects(objects), m_Type(Type::OBJECT)
{}

Object::Object(std::vector<std::shared_ptr<Object>> array)
: m_Array(array), m_Type(Type::ARRAY)
{}

Object::~Object() {}

template <> std::string Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to std::string.");
    return m_Scalar;
}

template <> int Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to int.");
    try {
        return std::stoi(m_Scalar);
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to int.");
    }
}

template <> double Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to double.");
    try {
        return std::stod(m_Scalar);
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to double.");
    }
}

template <> bool Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to boolean.");
    if (m_Scalar == "yes")
        return true;
    else if (m_Scalar == "no")
        return false;
    throw std::runtime_error("Invalid conversion of object to boolean.");
}

template <> Date Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to date.");
    try {
        return Date(m_Scalar);
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to date.");
    }
}

template <typename T> T Object::As() const {
    if (m_Type != Type::SCALAR)
        throw std::runtime_error("Invalid conversion of object to " + std::string(typeid(T).name()));
    try {
        return (T) m_Scalar;
    }
    catch (std::exception& e) {
        throw std::runtime_error(std::string(e.what()) + " Invalid conversion of object to " + std::string(typeid(T).name()));
    }
}

template <typename T> std::vector<T> Object::AsArray() const {
    if (m_Type != Type::ARRAY)
        throw std::runtime_error("Invalid conversion of object to array of " + std::string(typeid(T).name()));
    return std::vector<T>{};
}