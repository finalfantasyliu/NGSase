//
// Created by 劉軒豪 on 2024/1/31.
//

#include "mysql_database.h"

std::mutex mysql_database::initMutex;

mysql_database::mysql_database(std::string host, std::string username, std::string password, std::string database)
        : host{host}, username{username}, password{password}, database{database} {
    this->connection = nullptr;

}

mysql_database::~mysql_database() {
    mysql_close(this->connection);
}

bool mysql_database::connection_establish(int port, const char *unix_socket, CONNECT_SETTING setting) {

    std::unique_lock<std::mutex> initialzed_lock(initMutex);
    if (!this->connection)
        this->connection = mysql_init(nullptr);
    initialzed_lock.unlock();
    MYSQL *connection_status = nullptr;
    if (this->connection == nullptr) {
        std::cout << "initialized faild" << std::endl;
    }


    if (this->database.empty())

        connection_status = mysql_real_connect(this->connection, host.c_str(), username.c_str(), password.c_str(),
                                               nullptr,
                                               port, unix_socket, setting);
    else
        connection_status = mysql_real_connect(this->connection, host.c_str(), username.c_str(), password.c_str(),
                                               database.c_str(), port, unix_socket, setting);

    if (connection_status) {
        connection_established = true;
        this->unixSocket = unix_socket;
        this->port = port;
        return true;
    } else {
        connection_established = false;
        mysql_close(this->connection);
        this->connection = nullptr;
        std::cout << "connection establish failed" << std::endl;
        std::cout << "Please check your parameter" << std::endl;
        return false;
    }

}

mysql_result *mysql_database::query(std::string query_string) {
    mysql_result *result = new mysql_result;
    CRUD query_type = ToEnum(query_string);
    int query_status = 0;

    if (this->connection_established == false) {
        std::cout << "The connection isn't established" << std::endl;
        result->errorMSG = "The connection isn't established";
        return result;
    }

    if (query_type == CRUD::ERROR) {
        std::cout << "Theres is no CRUD string in your query_string" << std::endl;
        result->errorMSG = "Theres is no CRUD string in your query_string";
        return result;
    }


    result->crud = query_type;
    query_status = mysql_query(this->connection, query_string.c_str());
    if (!query_status) {
        switch (query_type) {
            case CRUD::CREATE: {
                if (query_string.find("database") != std::string::npos ||
                    query_string.find("DATABASE") != std::string::npos) {
                    result->affectRow = mysql_affected_rows(this->connection);
                    std::cout << "database creation succeed" << std::endl;

                    if (this->database.empty()) {
                        std::cout << "please reconnect new creation of database" << std::endl;
                        mysql_close(this->connection);
//                        mysql_thread_init();
//                        mysql_init(this->connection);
//                        mysql_thread_end();
                        this->connection_established = false;
                        return result;
                        break;

                    }
                    return result;
                    break;

                } else {
                    result->affectRow = mysql_affected_rows(this->connection);
                    std::cout << "table creation succeed" << std::endl;
                    return result;
                    break;
                }

            }
            case CRUD::INSERT: {
                result->affectRow = mysql_affected_rows(this->connection);
                std::cout << "INSERT succeed" << std::endl;
                return result;
                break;
            }
            case CRUD::SELECT: {

                result->result = mysql_store_result(this->connection);
                result->affectRow = mysql_num_rows(result->result);
                std::cout << "SELECT succeed" << std::endl;
                return result;
                break;
            }
            case CRUD::UPDATE: {
                result->affectRow = mysql_affected_rows(this->connection);
                std::cout << "UPDATE succeed" << std::endl;
                return result;
                break;

            }
            case CRUD::DELETE: {
                result->affectRow = mysql_affected_rows(this->connection);
                std::cout << "DELETE succeed" << std::endl;
                return result;
                break;

            }
            default: {
                std::cout << "Undefined behaviour" << std::endl;
                result->errorMSG = "The operation haven't be included in CRUD";
                return result;
                break;
            }


        }
    } else {
        //這邊記得要另外存取errorMSG，因為只要一rollback，memory也一併刪除
        std::string errorMSG(mysql_error(this->connection));
        result->errorMSG = errorMSG;
        if (transactionSwitch && commitStatus == MANUAL) {
            mysql_query(this->connection, "ROLLBACK");
            std::cout << "reset to default" << std::endl;
            autoCommitSwitch(true);
        }
        switch (query_type) {
            case CRUD::CREATE: {
                if (query_string.find("database") != std::string::npos ||
                    query_string.find("DATABASE") != std::string::npos) {
                    std::cout << "database creation failed" << std::endl;
                    std::cout << "ERROR: " << result->errorMSG << std::endl;
                    return result;

                    break;

                } else {
                    std::cout << "table creation failed" << std::endl;
                    std::cout << "ERROR: " << result->errorMSG << std::endl;
                    return result;
                    break;
                }
            }
            case CRUD::INSERT: {
                std::cout << "INSERT failed" << std::endl;
                std::cout << "ERROR: " << result->errorMSG << std::endl;
                return result;
                break;
            }
            case CRUD::SELECT: {
                std::cout << "SELECT failed" << std::endl;
                std::cout << "ERROR: " << result->errorMSG << std::endl;
                return result;
                break;
            }
            case CRUD::UPDATE: {
                std::cout << "UPDATE failed" << std::endl;
                std::cout << "ERROR: " << result->errorMSG << std::endl;
                return result;
                break;
            }
            case CRUD::DELETE: {
                std::cout << "DELETE failed" << std::endl;
                std::cout << "ERROR: " << result->errorMSG << std::endl;
                return result;
                break;
            }
            default: {
                std::cout << "Undefined behaviour" << std::endl;
                result->errorMSG = "The operation haven't be included in CRUD";
                return result;
                break;

            }

        }
    }


}

//autocommit開關 true=打開autocommit false=關閉autocommit
bool mysql_database::autoCommitSwitch(bool enable_disable) {
    //用一個commitModeChangeCheck來確認
    int commitModeChangeCheck = 0;
    if (!connection_established)
        return false;
    if (enable_disable) {
        commitModeChangeCheck = mysql_autocommit(this->connection, 1);//1等於開啟autocommit
        //若切換失敗會不等於0
        if (commitModeChangeCheck != 0) {
            std::cout << "Enable autocommit failed" << std::endl;
            std::cout << "The current state of autocommit is "
                      << ((commitStatus == AUTOCOMMIT) ? "auto commit" : "manual") << std::endl;
            return false;

        } else {
            std::cout << "Enable autocommit succeed" << std::endl;
            //更改commitStatus，讓使用者知道現在處於哪個狀態
            commitStatus = AUTOCOMMIT;
            return true;
        }
    } else {
        commitModeChangeCheck = mysql_autocommit(this->connection, 0);//0等於關閉autocommit
        if (commitModeChangeCheck != 0) {
            std::cout << "Disable autocommit failed" << std::endl;
            std::cout << "The current state of autocommit is "
                      << ((commitStatus == AUTOCOMMIT) ? "auto commit" : "manual") << std::endl;
            return false;

        } else {
            std::cout << "Disable autocommit succeed" << std::endl;
            commitStatus = MANUAL;
            return true;
        }
    }


}

bool mysql_database::startTransaction() {
    //確認連線是否正確連接
    if (!this->connection_established) {
        std::cout << "The connection hasn't been established" << std::endl;
        return false;
    }
//若commitStatus為autocommit切換為manual
    if (this->commitStatus == AUTOCOMMIT) {
        bool commitStatusChangeCheck = autoCommitSwitch(false);
        if (!commitStatusChangeCheck) {
            std::cout << "Can't start transaction, disable autocommit failed" << std::endl;
            return false;
        }
    }
//確認start transaction是否開啟成功
    if (mysql_query(this->connection, "START TRANSACTION") != 0) {
        std::cout << mysql_error(this->connection) << std::endl;
        return false;
    } else {
        std::cout << "transaction started" << std::endl;
        transactionSwitch = true;
        return true;

    }
}

bool mysql_database::commitTransaction() {
    if (!this->connection_established) {
        std::cout << "The connection hasn't been established" << std::endl;
        return false;
    }

    if (this->commitStatus == AUTOCOMMIT || transactionSwitch == false) {
        std::cout << "The transaction haven't been initialized" << std::endl;
        return false;
    }

    if (mysql_query(this->connection, "COMMIT") != 0) {
        std::cout << "Error on commit transaction" << mysql_error(this->connection) << std::endl;
        return false;
    } else {
        std::cout << "commit succeed" << std::endl;
        std::cout << "reset to default" << std::endl;
        transactionSwitch = false;
        autoCommitSwitch(true);
        return true;

    }

}



