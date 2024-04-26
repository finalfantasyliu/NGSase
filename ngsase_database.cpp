//
// Created by 劉軒豪 on 2024/1/31.
//

#include "ngsase_database.h"

bool ngsase_database::blueprint = false;

ngsase_database::ngsase_database(std::string host, std::string username, std::string password, int port,
                                 const char *unix_socket, CONNECT_SETTING setting) : mysql_database(host, username,
                                                                                                    password),
                                                                                     port{port},
                                                                                     unix_socket{unix_socket},
                                                                                     setting(setting) {
    if (!blueprint) {
        std::lock_guard<std::mutex> blueprint_mutexlock(blueprint_mutex);
        if (!blueprint) {
            std::cout << "check ngase database" << std::endl;
            if (connection_establish(this->port, this->unix_socket, this->setting)) {
                mysql_result *result = query("SELECT *FROM information_schema.SCHEMATA WHERE SCHEMA_NAME = 'ngsase';");
                std::vector<std::string> command;
                command.push_back(
                    "CREATE TABLE IF NOT EXISTS users (email VARCHAR(255) NOT NULL PRIMARY KEY,password VARCHAR(50) NOT NULL);");
                command.push_back(
                    "CREATE TABLE IF NOT EXISTS pipelines("
                    "pipeline_name VARCHAR(255) NOT NULL,"
                    " tool VARCHAR(255) NOT NULL,"
                    " setting JSON NOT NULL,"
                    " step INT CHECK (step>0) NOT NULL,"
                    " email VARCHAR(255) NOT NULL,"
                    " PRIMARY KEY(pipeline_name,email,step),"
                    " FOREIGN KEY(email) REFERENCES users(email),"
                    " INDEX idx_pipeline_step_email (pipeline_name, step, email)"
                    ");");
                command.push_back(
                    "CREATE TABLE IF NOT EXISTS projects("
                    "uuid VARCHAR(36) NOT NULL,"
                    "project_name VARCHAR(255) NOT NULL,"
                    "pipeline_name VARCHAR(255) NOT NULL,"
                    "paired_end BOOLEAN NOT NULL,"
                    "input_directory TEXT NOT NULL,"
                    "output_directory TEXT NOT NULL,"
                    "project_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                    "finished BOOLEAN NOT NULL,"
                    "email VARCHAR(255) NOT NULL,"
                    "PRIMARY KEY(project_name,pipeline_name,email),"
                    "FOREIGN KEY(pipeline_name,email) REFERENCES pipelines(pipeline_name,email));");
                command.push_back(
                    "CREATE TABLE IF NOT EXISTS genomeIndexs("
                    "index_name VARCHAR(255) NOT NULL,"
                    "index_directory TEXT NOT NULL,"
                    "tool VARCHAR(255) NOT NULL,"
                    "email VARCHAR(255) NOT NULL,"
                    "PRIMARY KEY(index_name,tool,email),"
                    " FOREIGN KEY(email) REFERENCES users(email));");
                command.push_back(
                    "CREATE TABLE IF NOT EXISTS gtfs("
                    "gtf_name VARCHAR(255) NOT NULL,"
                    "gtf_directory TEXT NOT NULL,"
                    "email VARCHAR(255) NOT NULL,"
                    "PRIMARY KEY(gtf_name,email),"
                    "FOREIGN KEY(email) REFERENCES users(email));");
                if (result->affectRow == 0 && result->errorMSG.empty()) {
                    delete result;
                    result = query("CREATE DATABASE ngsase;");

                    if (!result->errorMSG.empty()) {
                        std::cout << "ERROR: " << result->errorMSG;
                        delete result;
                        result = nullptr;
                        return;
                    }

                    delete result;
                    result = nullptr;
                    this->database = "ngsase";
                    if (connection_establish(this->port, this->unix_socket, setting)) {
                        for (auto &word : command) {
                            result = query(word);
                            if (!result->errorMSG.empty()) {
                                std::cout << "ERROR: " << result->errorMSG << std::endl;
                                delete result;
                                result = nullptr;
                                return;
                                break;
                            } else {
                                delete result;
                                result = nullptr;
                            }
                        }
                    }
                    blueprint = true;
                } else if (result->affectRow == 1 && result->errorMSG.empty()) {
                    delete result;
                    result = nullptr;
                    mysql_close(this->connection);
                    this->connection = nullptr;
                    this->database = "ngsase";
                    if (connection_establish(port, unix_socket, setting)) {
                        for (auto &word : command) {
                            result = query(word);
                            if (!result->errorMSG.empty()) {
                                std::cout << "ERROR: " << result->errorMSG << std::endl;
                                delete result;
                                result = nullptr;
                                return;
                                break;
                            } else {
                                delete result;
                                result = nullptr;
                            }
                        }
                    }
                    blueprint = true;
                } else {
                    std::cout << "ERROR: " << result->errorMSG << std::endl;
                    delete result;
                    result = nullptr;
                }
            }
        } else {
            this->database = "ngsase";

            std::cout << "the ngase database is already existed and initialized" << std::endl;
            if (connection_establish(this->port, this->unix_socket, this->setting))
                std::cout << "ngsase database is connected properly" << std::endl;
            else
                std::cout << "ngsase database isn't connected properly" << std::endl;
        }

    } else {
        std::cout << "ngase database is already existed and initialized earlier" << std::endl;
        this->database = "ngsase";
        if (connection_establish(this->port, this->unix_socket, this->setting))
            std::cout << "ngsase database is connected properly" << std::endl;
        else
            std::cout << "ngsase database isn't connected properly" << std::endl;
    }
}
