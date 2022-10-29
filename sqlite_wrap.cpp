
#include "sqlite_wrap.h"

#include <iostream>
#include <sqlite3.h>

namespace SqExp
{
void cleanup_db(sqlite3 *db)
{
    if (db == nullptr)
        return;
    int rc = sqlite3_close(db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Couldn't close database. Something is still using it.\n";
    }
}

void cleanup_statement(sqlite3_stmt *statment)
{
    // allowed to call on null
    sqlite3_finalize(statment);
    // will always succeed so no error handling.
}

SqliteWrap::SqliteWrap(const std::string &path)
    : _db_path(path),
      _db(nullptr, cleanup_db)
{
    // Create database
    sqlite3 *db;
    // Read/Write + Create if it doesn't exist
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(_db_path.c_str(), &db, flags, nullptr);

    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);

        std::cerr << "Failed to open database: '" << _db_path << "'\n";
        // since we cleaned up what we allocated,
        // we can throw if we want
    }
    else
    {
        _db.reset(db);
    }
}

deleter_unique_ptr<sqlite3_stmt> SqliteWrap::make_statement(
        const std::string& sql,
        const std::map<std::string, bindable>& bindMap)
{
    sqlite3_stmt *statement_holder;

    int rc = sqlite3_prepare_v2(_db.get(), sql.c_str(), sql.length(), &statement_holder, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to compile sql - " << rc << "\n";
        return {};
    }

    // Statement will be cleaned up automatically
    deleter_unique_ptr<sqlite3_stmt> statement(statement_holder, cleanup_statement);

    for (auto& binding: bindMap)
    {
        int index = sqlite3_bind_parameter_index(statement.get(), binding.first.c_str());
        if (index == 0)
        {
            std::cerr << "Failed to find index for parameter " << binding.first << "\n";
            continue;
        }
        auto value = binding.second;
        int rc = -1;
        if (std::holds_alternative<int>(value))
        {
            rc = sqlite3_bind_int(statement.get(), index, std::get<int>(value));
        }
        else if (std::holds_alternative<double>(value))
        {
            rc = sqlite3_bind_double(statement.get(), index, std::get<double>(value));
        }
        else if (std::holds_alternative<std::string>(value))
        {
            auto& str = std::get<std::string>(value);
            rc = sqlite3_bind_text(
                statement.get(),
                index,
                str.c_str(),
                str.length(),
                SQLITE_STATIC); // strings will live till we finalize
        }
        if (rc != SQLITE_OK)
        {
            std::cerr << "Failed to bind sql paramater " << rc 
                << " (" << binding.first << ")\n";
        }
    }
    return statement;
}

void SqliteWrap::execute(
    const std::string &sql, const std::map<std::string, bindable> &bindMap)
{
    auto statement = make_statement(sql, bindMap);
    int rc = sqlite3_step(statement.get());
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute sql: " << rc << "\n";
    }
}

bool SqliteWrap::next_row(sqlite3_stmt* unowned_statement)
{
    int rc = sqlite3_step(unowned_statement);
    return rc == SQLITE_ROW;
}


void SqliteWrap::get_row_column(sqlite3_stmt* unowned_statement, int index, int* value)
{
    *value = sqlite3_column_int(unowned_statement, index);
}

void SqliteWrap::get_row_column(sqlite3_stmt* unowned_statement, int index, double* value)
{
    *value = sqlite3_column_double(unowned_statement, index);
}

void SqliteWrap::get_row_column(sqlite3_stmt* unowned_statement, int index, std::string* value)
{
    const unsigned char* str = sqlite3_column_text(unowned_statement, index);
    int bytes = sqlite3_column_bytes(unowned_statement, index);

    value->insert(0, (const char*)str, bytes);
}

}