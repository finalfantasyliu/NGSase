//
// Created by 劉軒豪 on 2024/1/31.
//

#ifndef NGSASE_MYSQL_DATABASE_API_H
#define NGSASE_MYSQL_DATABASE_API_H

#include <mysql.h>

#include <iostream>
#include <mutex>
#include <string>


enum CONNECT_SETTING {
    ROW_FOUND = CLIENT_FOUND_ROWS,
    SIGPIPE_IGN = CLIENT_IGNORE_SIGPIPE,
    MULTI_RESULT = CLIENT_MULTI_RESULTS,
    MULTI_STATEMENT = CLIENT_MULTI_STATEMENTS,
};
enum class CRUD {
    CREATE,
    INSERT,
    SELECT,
    UPDATE,
    DELETE,
    ERROR
};
enum COMMIT_STATUS {
    AUTOCOMMIT,
    MANUAL
};

class mysql_result {
   public:
    int affectRow = 0;
    CRUD crud;
    MYSQL_RES *result = nullptr;
    std::string errorMSG;
    ~mysql_result() {
        if (result)
            mysql_free_result(result);
    }
};

class mysql_database {
   public:
    mysql_database(std::string host, std::string username, std::string password, std::string database = "");

    ~mysql_database();

    bool connection_establish(int port, const char *unix_socket, CONNECT_SETTING setting);

    mysql_result *query(std::string query_string);

    bool autoCommitSwitch(bool enable);

    bool startTransaction();

    bool commitTransaction();

   public:
    std::string host;
    std::string username;
    std::string password;
    std::string database;
    int port;
    const char* unixSocket;
    MYSQL *connection;
    COMMIT_STATUS commitStatus = AUTOCOMMIT;
    bool transactionSwitch = false;
    bool connection_established = false;
    static std::mutex initMutex;
};

inline CRUD ToEnum(std::string &query_string) {
    std::string temp_string = query_string;
    std::transform(temp_string.begin(), temp_string.end(), temp_string.begin(), ::toupper);
    if (temp_string.find("CREATE") != std::string::npos)
        return CRUD::CREATE;
    else if (temp_string.find("INSERT") != std::string::npos)
        return CRUD::INSERT;
    else if (temp_string.find("SELECT") != std::string::npos)
        return CRUD::SELECT;
    else if (temp_string.find("UPDATE") != std::string::npos)
        return CRUD::UPDATE;
    else if (temp_string.find("DELETE") != std::string::npos)
        return CRUD::DELETE;
    else
        return CRUD::ERROR;
}

#endif  // SINGLE_THREAD_MYSQL_DATABASE_H
