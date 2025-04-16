#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <map>

namespace Jomini {

    class Object;
    class Parser;

    struct Date {
        int year;
        int month;
        int day;
    
        Date(int y, int m, int d) : year(y), month(m), day(d) {}
    
        Date(const std::string& str) {
            std::string buffer = "";
            int parts[3];
            int n = 0;
    
            for (int i = 0; i < str.size(); i++) {
                if(str[i] != '.') {
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

    enum Type {
        SCALAR,
        OBJECT,
        ARRAY
    };

    class Object {
        public:
            Object();
            Object(std::string scalar);
            Object(std::map<std::string, std::shared_ptr<Object>> objects);
            Object(std::vector<std::shared_ptr<Object>> array);
            ~Object();

            template <typename T> T As() const;
            template <typename T> std::vector<T> AsArray() const;

        private:
            union {
                std::string m_Scalar;
                std::map<std::string, std::shared_ptr<Object>> m_Objects;
                std::vector<std::shared_ptr<Object>> m_Array;
            };
            Type m_Type;
    };

    class Parser {
        public:
            Parser();

        private:
    };
}