//
// Created by 劉軒豪 on 2024/1/31.
//

#ifndef NGSASE_DATABASE_H
#define NGSASE_DATABASE_H


#include "mysql_database.h"
#include <mutex>
#include <string>
#include <vector>


class ngsase_database: public mysql_database {
public:
    ngsase_database(std::string host, std::string username, std::string password,int port,const char *unix_socket,CONNECT_SETTING setting);
public:
    static bool blueprint;
    std::mutex blueprint_mutex;
    int port;
    const char* unix_socket;
    CONNECT_SETTING setting;




};


#endif //SINGLE_THREAD_NGSASE_DATABAS_H
