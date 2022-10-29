
#include "sqlite_wrap.h"
#include <iostream>

using namespace SqExp;
int main()
{
    auto db = SqliteWrap(":memory");
    db.execute("CREATE TABLE t1(one, two);", {});
    db.execute(
        "CREATE TABLE IF NOT EXISTS things"
        "(id TEXT, count INT, score REAL);",
        {}
    );
    db.execute(
        "INSERT INTO things (id, count, score) VALUES "
        "(@id, @c, @s);",
        {{"@id", "bat"}, {"@c", 1}, {"@s", 0.2}}
    );
    db.execute(
        "INSERT INTO things (id, count, score) VALUES "
        "(@id, @c, @s);",
        {{"@id", "rice"}, {"@c", 5000}, {"@s", 0.1}}
    );
    db.execute(
        "INSERT INTO things (id, count, score) VALUES "
        "(@id, @c, @s);",
        {{"@id", "hat"}, {"@c", 6}, {"@s", 0.8}}
    );

    auto data = db.get_data<std::string, int, double>(
        "SELECT id, count, score FROM things;",
        {}
    );
    for (const auto& [i, s, c]: data)
        std::cout << i << " " << s << " " << c << std::endl;
    return 0;
}