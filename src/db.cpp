#include "db.h"
#include <libpq-fe.h>
#include <iostream>
#include <cstdlib>

Database::Database() : conn_(nullptr)
{
}

Database::~Database()
{
    if (conn_)
    {
        PQfinish(conn_);
    }
}

bool Database::connect()
{
    const char* host = std::getenv("DB_HOST");
    const char* port = std::getenv("DB_PORT");
    const char* db = std::getenv("DB_NAME");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASSWORD");

    conninfo_ = 
        "host=" + std::string(host ? host : "localhost") +
        " port=" + std::string(port ? port : "5432") +
        " dbname=" + std::string(db ? db : "postgres") +
        " user=" + std::string(user ? user : "postgres") +
        " password=" + std::string(pass ? pass : "");

    conn_ = PQconnectdb(conninfo_.c_str());

    if (PQstatus(conn_) != CONNECTION_OK)
    {
        std::cerr << "DB error: " << PQerrorMessage(conn_) << std::endl;
        return false;
    }

    std::cout << "Connected to DB" << std::endl;
    return true;
}

std::optional<User> Database::find_user(const std::string &email, const std::string &username)
{
    const char* params[2] = { email.c_str(), username.c_str() };

    PGresult* res = PQexecParams(
        conn_,
        "SELECT id, username, email, role FROM users WHERE email=$1 OR username=$2",
        2,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "SELECT failed" << std::endl;
        PQclear(res);
        return std::nullopt;
    }

    int rows = PQntuples(res);

    if (rows == 0) {
        PQclear(res);
        return std::nullopt;
    }
    
    User user;
    user.id       = PQgetvalue(res, 0, 0);
    user.username = PQgetvalue(res, 0, 1);
    user.email    = PQgetvalue(res, 0, 2);
    user.role     = PQgetvalue(res, 0, 3);

    PQclear(res);
    return user;
}

std::optional<std::string> Database::insert_user(const User &user, const std::string &password_hash)
{
    const char* params[8] = {
        user.username.c_str(),
        user.email.c_str(),
        user.phone.c_str(),
        password_hash.c_str(),
        user.role.c_str(),
        user.company_name ? user.company_name->c_str() : nullptr,
        user.inn ? user.inn->c_str() : nullptr,
        user.metadata ? user.metadata->c_str() : nullptr
    };

    PGresult* res = PQexecParams(
        conn_,
        "INSERT INTO users "
        "(username,email,phone,password_hash,role,company_name,inn,metadata) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7,$8) "
        "RETURNING id",
        8,
        nullptr,
        params,
        nullptr,
        nullptr,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK &&
        PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Insert error: " << PQerrorMessage((PGconn*)conn_) << std::endl;
        PQclear(res);
        return std::nullopt;
    }

    std::string id = PQgetvalue(res, 0, 0);

    PQclear(res);
    return id;
}
