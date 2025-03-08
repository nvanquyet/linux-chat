#ifndef DB_STATEMENT_H
#define DB_STATEMENT_H

#include <mysql/mysql.h>
#include <stdbool.h>

typedef struct DbStatement DbStatement;

struct DbStatement{
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND *binds;
    unsigned long param_count;
    bool is_bound;
};

DbStatement* db_prepare(const char* query);
bool db_bind_string(DbStatement* stmt, int index, const char* value);
bool db_execute(DbStatement* stmt);

#endif