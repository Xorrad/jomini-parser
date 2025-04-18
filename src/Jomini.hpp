#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <map>
#include <list>
#include <utility>
#include <fstream>

namespace Jomini {

    //////////////////////////////////////////////////////////
    //                 Forward Declarations                 //
    //////////////////////////////////////////////////////////

    struct Date;
    struct Color;

    template <typename K, typename V>
    class OrderedMap;

    class Object;
    class Parser;

    //////////////////////////////////////////////////////////
    //           Custom Data Types (Date, Color...)         //
    //////////////////////////////////////////////////////////

    struct Date {
        int year;
        int month;
        int day;
    
        Date() : year(0), month(0), day(0) {}
        Date(int y, int m, int d) : year(y), month(m), day(d) {}
    
        Date(const std::string& str) {
            std::string buffer = "";
            int parts[3];
            int n = 0;
    
            for (int i = 0; i < str.size(); i++) {
                if (str[i] != '.') {
                    buffer += str[i];
                    continue;
                }
                if (n >= 3)
                    throw std::invalid_argument("Invalid date format.");
                try {
                    parts[n++] = std::stoi(buffer);
                }
                catch (std::exception& e) {
                    throw std::invalid_argument("Invalid date number format.");
                }
            }
    
            if (n != 3)
                throw std::invalid_argument("Invalid date format.");
        
            year = parts[0];
            month = parts[1];
            day = parts[2];
        }
    
        operator std::string() const {
            return std::to_string(year) + "." + std::to_string(month) + "." + std::to_string(day);
        }
        
        bool operator ==(const Date& other) const {
            return year == other.year && month == other.month && day == other.day;
        }
        
        bool operator <=(const Date& other) const {
            return !(*this > other);
        }
        
        bool operator >=(const Date& other) const {
            return !(*this < other);
        }
    
        bool operator <(const Date& other) const {
            return year < other.year
                || (year == other.year && month < other.month)
                || (year == other.year && month == other.month && day < other.day);
        }
        
        bool operator >(const Date& other) const {
            return year > other.year
                || (year == other.year && month > other.month)
                || (year == other.year && month == other.month && day > other.day);
        }
    };

    //////////////////////////////////////////////////////////
    //                     Utilities                        //
    //////////////////////////////////////////////////////////

    template <typename K, typename V>
    class OrderedMap {
        public:
            OrderedMap() {}

            OrderedMap(const std::map<K, V>& entries) {
                m_Items.resize(entries.size());
                for (auto [key, value] : entries)
                    this->insert(key, value);
            }

            void insert(const K& key, const V& value) {
                if (m_Index.find(key) == m_Index.end()) {
                    m_Items.emplace_back(key, value);
                    m_Index[key] = --m_Items.end();
                } else {
                    m_Index[key]->second = value;
                }
            }

            V& at(const K& key) {
                if (m_Index.find(key) == m_Index.end()) {
                    throw std::out_of_range("key not found");
                }
                return m_Index[key]->second;
            }

            V& operator[](const K& key) {
                if (m_Index.find(key) == m_Index.end()) {
                    m_Items.emplace_back(key, V());
                    m_Index[key] = --m_Items.end();
                }
                return m_Index[key]->second;
            }

            const V& operator[](const K& key) const {
                static V default_value{};
                auto it = m_Index.find(key);
                return (it != m_Index.end()) ? it->second->second : default_value;
            }

            typename std::list<std::pair<K, V>>::iterator begin() {
                return m_Items.begin();
            }

            typename std::list<std::pair<K, V>>::iterator end() {
                return m_Items.end();
            }

            typename std::list<std::pair<K, V>>::const_iterator begin() const {
                return m_Items.cbegin();
            }

            typename std::list<std::pair<K, V>>::const_iterator end() const {
                return m_Items.cend();
            }

            std::size_t size() const {
                return m_Items.size();
            }

            bool empty() const {
                return m_Items.size() == 0;
            }

            void erase(const K& key) {
                auto it = m_Index.find(key);
                if (it != m_Index.end()) {
                    m_Items.erase(it->second);
                    m_Index.erase(it);
                }
            }

            typename std::list<std::pair<K, V>>::iterator find(const K& key) {
                auto it = m_Index.find(key);
                return (it != m_Index.end()) ? it->second : m_Items.end();
            }

            bool contains(const K& key) const {
                return m_Index.count(key) > 0;
            }

            std::vector<K> keys() const {
                std::vector<K> keys;
                for (auto [key, value] : m_Items)
                    keys.push_back(key);
                return keys;
            }

            void clear() {
                m_Items.clear();
                m_Index.clear();
            }

        private:
            using ListIterator = typename std::list<std::pair<K, V>>::iterator;
            std::list<std::pair<K, V>> m_Items;
            std::map<K, ListIterator> m_Index;
    };

    //////////////////////////////////////////////////////////
    //                  Jomini Objects                      //
    //////////////////////////////////////////////////////////

    enum class Type {
        SCALAR,
        OBJECT,
        ARRAY
    };

    enum class Operator {
        EQUAL,
        LESS,
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
        NOT_EQUAL,
        NOT_NULL,
    };
    const std::map<Operator, std::string> OperatorsLabels = {
        { Operator::EQUAL, "=" },
        { Operator::LESS, "<" },
        { Operator::LESS_EQUAL, "<=" },
        { Operator::GREATER, ">" },
        { Operator::GREATER_EQUAL, ">=" },
        { Operator::NOT_EQUAL, "!=" },
        { Operator::NOT_NULL, "?=" },
    };

    class Object {
        public:
            Object();
            Object(int scalar);
            Object(double scalar);
            Object(bool scalar);
            Object(const std::string& scalar);
            Object(const Date& scalar);
            Object(const OrderedMap<std::string, std::pair<Operator, std::shared_ptr<Object>>>& objects);
            Object(const std::vector<std::shared_ptr<Object>>& array);
            Object(const Object& object);
            Object(const std::shared_ptr<Object>& object);
            ~Object();

            std::shared_ptr<Object> Copy() const;

            template <typename T> T As() const;
            template <typename T> std::vector<T> AsArray() const;

            std::shared_ptr<Object> Get(const std::string& key);
            Operator GetOperator(const std::string& key);

            template <typename T> T Put(std::string key, T value, Operator op = Operator::EQUAL);

            std::string& GetScalar();
            OrderedMap<std::string, std::pair<Operator, std::shared_ptr<Object>>>& GetEntries();
            std::vector<std::shared_ptr<Object>>& GetValues();

        private:
            union {
                std::string m_Scalar;
                OrderedMap<std::string, std::pair<Operator, std::shared_ptr<Object>>> m_Objects;
                std::vector<std::shared_ptr<Object>> m_Array;
            };
            Type m_Type;
    };
    
    //////////////////////////////////////////////////////////
    //                      Parser                          //
    //////////////////////////////////////////////////////////

    class Parser {
        public:
            Parser();

            std::shared_ptr<Object> ParseFile(const std::string& filePath);
            std::shared_ptr<Object> ParseString(const std::string& content);
            std::shared_ptr<Object> Parse(std::istream& stream, int depth = 0);

        private:
            std::string m_FilePath;
            std::vector<std::string> m_Lines;
            int m_CurrentLine;
    };

    std::shared_ptr<Object> ParseFile(const std::string& filePath);
    std::shared_ptr<Object> ParseString(const std::string& content);
    std::shared_ptr<Object> Parse(std::istream& stream);
}