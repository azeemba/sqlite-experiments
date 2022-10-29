
#include <memory>
#include <string>
#include <functional>
#include <variant>
#include <map>
#include <tuple>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

namespace SqExp
{
template<class T>
using deleter_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;
using bindable = std::variant<int, double, std::string>;


class SqliteWrap
{
    std::string _db_path; // no fancy URI, just path

    deleter_unique_ptr<sqlite3> _db;
    deleter_unique_ptr<sqlite3_stmt> make_statement(
        const std::string& sql,
        const std::map<std::string, bindable>& bindMap);
    
    bool next_row(sqlite3_stmt* unowned_statement);
    void get_row_column(sqlite3_stmt* unowned_statement, int index, int* value);
    void get_row_column(sqlite3_stmt* unowned_statement, int index, double* value);
    void get_row_column(sqlite3_stmt* unowned_statement, int index, std::string* value);

    template<typename T>
    T get_row_column(sqlite3_stmt* unowned_statement, int index)
    {
        T value;
        get_row_column(unowned_statement, index, &value);
        return value;
    }

    template<int N = 0>
    std::tuple<> get_row_data(sqlite3_stmt* unowned_statement, int start_index)
    {
        return std::make_tuple();
    }

    template<int N, typename T, typename... Ts>
    std::tuple<T, Ts...> get_row_data(
        sqlite3_stmt* unowned_statement, int start_index)
    {
        return std::tuple_cat(
            std::make_tuple(get_row_column<T>(unowned_statement, start_index)),
            get_row_data<N-1, Ts...>(unowned_statement, start_index+1));
    }
    
public:
    SqliteWrap(const std::string& path);

    void execute(
        const std::string& sql,
        const std::map<std::string, bindable>& bindMap);
    
    
    template<typename... Ts>
    std::vector<std::tuple<Ts...>> get_data(
        const std::string& sql,
        const std::map<std::string, bindable>& bindMap)
    {
        std::vector<std::tuple<Ts...>> results;
        auto statement = make_statement(sql, bindMap);
        while (next_row(statement.get()))
        {
            results.emplace_back(
                get_row_data<std::tuple_size_v<std::tuple<Ts...>>, Ts...>(statement.get(), 0)
            );
        }

        return results;
    }
};
}