cmake_minimum_required(VERSION 3.27)
project(NGSase)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        /opt/homebrew/opt/mysql/include/mysql/
        /opt/homebrew/include)



add_executable(NGSase main.cpp
        contentType_op.h
        contentType_op.cpp
        http_request.h
        http_request.cpp
        http_response.h
        http_response.cpp
        mysql_database.h
        mysql_database.cpp
        ngsase_database.h
        ngsase_database.cpp
        ngsase_json.h
        ngsase_json.cpp
        ngsase_requestProc.h
        ngsase_requestProc.cpp
        reactor.h
        reactor.cpp
        thread_pool.h
        time_wheel.h
        time_wheel.cpp)

target_link_libraries(NGSase /opt/homebrew/opt/mysql/lib/libmysqlclient.dylib)