#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <map>
#include <unordered_map>
#include <list>
#include <utility>
#include <fstream>
#include <variant>
#include <optional>
#include <format>
#include <string_view>
#include <functional>
#include <ranges>

namespace Jomini {

    //////////////////////////////////////////////////////////
    //                 Forward Declarations                 //
    //////////////////////////////////////////////////////////

    struct Date;
    struct Color;

    class ObjectMap;

    class Object;
    class Parser;

    //////////////////////////////////////////////////////////
    //                  Jomini Object Types                 //
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

    enum class Flags {
        NONE  = 0,
        RGB   = 1 << 0,
        HSV   = 1 << 1,
        LIST  = 1 << 2,
        RANGE = 1 << 3,
    };
    
    Flags operator|(Flags a, Flags b);
    Flags operator&(Flags a, Flags b);
    Flags operator~(Flags a);
    Flags& operator|=(Flags& a, Flags b);
    Flags& operator&=(Flags& a, Flags b);
    
    //////////////////////////////////////////////////////////
    //           Custom Data Types (Date, Color...)         //
    //////////////////////////////////////////////////////////

    struct Date {
        int year;
        int month;
        int day;
    
        Date();
        Date(int y, int m, int d);
        Date(const std::string& str);
    
        operator std::string() const;
        
        bool operator ==(const Date& other) const;
        bool operator <=(const Date& other) const;
        bool operator >=(const Date& other) const;
        bool operator <(const Date& other) const;
        bool operator >(const Date& other) const;
    };

    //////////////////////////////////////////////////////////
    //                 Ordered Object Map                   //
    //////////////////////////////////////////////////////////

    struct StringHash {
        using is_transparent = void; 
        size_t operator()(const std::string& s) const { return std::hash<std::string>{}(s); }
        size_t operator()(const char* c) const { return std::hash<std::string_view>{}(c); }
        size_t operator()(std::string_view sv) const { return std::hash<std::string_view>{}(sv); }
    };

    class ObjectMap {
        public:
            using Value = std::pair<Operator, std::shared_ptr<Object>>;
            using Item = std::pair<std::string, Value>;
            using List = std::list<Item>;
            using Iterator = typename List::iterator;
            using ConstIterator = typename List::const_iterator;
            using IndexMap = std::unordered_map<std::string, Iterator, StringHash, std::equal_to<>>;

            // Initialized in the source file with (EQUAL, nullptr).
            static const Value s_DefaultValue;

            // Constructors
            ObjectMap();
            ObjectMap(const std::unordered_map<std::string, Value>& entries);

            // Modifiers
            void insert(const std::string& key, const Value& value);
            void insert(std::string_view key, const Value& value);
            void insert_missing(std::string_view key, const Value& value);
            template <typename K> void erase(const K& key);
            void clear();

            // Lookups
            template <typename K> Value& at(const K& key);
            template <typename K> const Value& at(const K& key) const;
            Value& operator[](std::string_view key);
            template <typename K> const Value& operator[](const K& key) const;
            template <typename K> Iterator find(const K& key);
            template <typename K> ConstIterator find(const K& key) const;
            template <typename K> bool contains(const K& key) const;

            // Iterators
            Iterator begin();
            Iterator end();
            ConstIterator begin() const;
            ConstIterator end() const;

            // Miscellaneous
            std::vector<std::string_view> keys() const;

            // Sizes and states
            std::size_t size() const;
            bool empty() const;
            void reserve(size_t size);

        private:
            List m_Items;
            IndexMap m_Index;
    };

    //////////////////////////////////////////////////////////
    //                  Jomini Objects                      //
    //////////////////////////////////////////////////////////

    using ObjectArray = std::vector<std::shared_ptr<Object>>;

    class Object {
        public:
            Object();
            Object(int scalar);
            Object(double scalar);
            Object(bool scalar);
            Object(const std::string& scalar);
            Object(const Date& scalar);
            Object(const ObjectMap& objects);
            Object(const ObjectArray& array);
            Object(const std::variant<std::string, ObjectMap, ObjectArray>& value);
            Object(const Object& object);
            Object(const std::shared_ptr<Object>& object);
            Object(const std::string_view& view);
            ~Object();

            Type GetType() const;
            bool Is(Type type) const;
            std::shared_ptr<Object> Copy() const;

            Flags GetFlags() const;
            bool HasFlag(Flags flag) const;
            void SetFlags(Flags flags);
            void SetFlag(Flags flag, bool enabled);

            void ConvertToArray();
            void ConvertToObject();

            template <typename T> T As() const;
            template <typename T> std::optional<T> AsOpt() const;
            template <typename T> T As(const T& defaultValue) const;

            template <typename T> std::vector<T> AsArray() const;
            template <typename T> std::optional<std::vector<T>> AsArrayOpt() const;
            template <typename T> std::vector<T> AsArray(const std::vector<T>& defaultValue) const;

            bool Contains(std::string_view key) const;
            std::shared_ptr<Object> Get(std::string_view key);
            Operator GetOperator(std::string_view key);
            
            template <typename T> void Push(T value, bool convertToArray = false);
            void Remove(std::string_view key);

            template <typename T> void Put(std::string_view key, T value, Operator op = Operator::EQUAL);
            template <typename T> void Merge(std::string_view key, T value, Operator op = Operator::EQUAL);
            template <typename T> void MergeUnsafe(std::string_view key, T value, Operator op = Operator::EQUAL);

            std::string& GetString();
            ObjectMap& GetMap();
            ObjectArray& GetArray();
            
            ObjectMap& GetMapUnsafe();
            ObjectArray& GetArrayUnsafe();

            std::string Serialize(uint depth = 0, bool isInline = false) const;

        private:
            std::string SerializeScalar(uint depth = 0) const;
            std::string SerializeObject(uint depth = 0, bool isInline = false) const;
            std::string SerializeArray(uint depth = 0) const;
            std::string SerializeArrayRange(const std::string& key, Operator op, uint depth = 0) const;

            std::variant<std::string, ObjectMap, ObjectArray> m_Value;
            Type m_Type;
            Flags m_Flags;
    };
    
    //////////////////////////////////////////////////////////
    //                      Reader                          //
    //////////////////////////////////////////////////////////
    
    class Reader {
        public:
            Reader();
            Reader(std::string filePath);
            ~Reader();

            void OpenFile(std::string filePath);
            void OpenString(std::string content);
            void Open(std::istream& stream);

            bool IsEmpty();
            char Read();
            char Peek();
            bool Match(char expected);

            std::string_view ReadUntil(const std::function<bool(char)>& predicate, bool includePrevious = false, bool includeLast = false);
            void SkipUntil(const std::function<bool(char)>& predicate);

            std::string_view GetView() const;
            std::string_view GetLine(uint line) const;

            uint GetCurrentLine() const;
            uint GetCurrentCursor() const;

        private:
            void IncrementLine();

            std::string m_Buffer;
            std::string_view m_View;

            uint m_CurrentLine;
            uint m_CurrentCursor;
            uint m_CurrentGlobalCursor;

    };

    
    //////////////////////////////////////////////////////////
    //                      Parser                          //
    //////////////////////////////////////////////////////////

    #define THROW_ERROR(error, cursorError, cursorOffset) this->ThrowError(error, cursorError, cursorOffset, __FILE__, __LINE__);

    class Parser {
        public:
            Parser();

            void ThrowError(const std::string& error, const std::string& cursorError, int cursorOffset, std::string sourceFile, int sourceFileLine);

            std::shared_ptr<Object> ParseFile(const std::string& filePath);
            std::shared_ptr<Object> ParseString(const std::string& content);

        private:
            std::shared_ptr<Object> Parse(int depth);

            std::string m_FilePath;
            Reader m_Reader;
            int m_PreviousLine;
            int m_PreviousCursor;
            int m_LastBraceLine;
    };

    std::shared_ptr<Object> ParseFile(const std::string& filePath);
    std::shared_ptr<Object> ParseString(const std::string& content);
}