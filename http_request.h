//
// Created by 劉軒豪 on 2024/1/31.
//

#ifndef NGSASE_HTTP_REQUEST_H
#define NGSASE_HTTP_REQUEST_H
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "contentType_op.h"

enum class Method {
    GET,
    POST,
    PUT,
    DELETE

};

class Multipart {
   public:
    ContentType type;
    std::string Disposition;
    std::string content;
};

class HTTPRequest {
   public:
    Method method;
    std::string URL;
    std::string version;
    std::string Host;
    std::string connection;
    ContentType contentType = NONE;
    std::vector<std::string> restHeaders;
    int contentLength = 0;
    int contentReadNumber = 0;
    std::string body;
    std::string fileLocation;
    std::string subDirectory;
    std::string boundary;
    std::ofstream outputFile;
    std::vector<Multipart> multipart;
    bool headerComplete = false;
    ~HTTPRequest() {
        if (outputFile.is_open())
            outputFile.close();
    }
};

// 讀取狀態
typedef enum check_state {
    CHECK_STATE_REQUESTLINE,  // 處於檢測requestline狀態 GET /text.html HTTP/1.1
    CHECK_STATE_HEADER,       // 處於檢測header line 狀態
    CHECK_STATE_BODY
} CheckState;

// 確認line是否完整
typedef enum line_state {
    LINE_OK,   // 確認讀到\r\n
    LINE_BAD,  // 確認完全沒有\r\n
    LINE_OPEN  // 還沒讀到\r\n
} LineState;

// http request 是否完整
typedef enum http_code {
    NO_REQUEST,        // request不完整，需繼續讀取
    GET_HEADER,        // 得到完整的request
    GET_REQUEST,       //
    BAD_REQUEST,       // request 語法錯誤
    FORBIDDEN_ERROR,   // 沒有讀取資料的權限
    INTERNAL_ERROR,    // 伺服器內部錯誤
    CLOSE_CONNECTION,  // client關閉連接 read=0
    LONG_REQUEST,
} HTTPCode;


LineState parse_line(char *buffer, int &check_index, int &read_index);
HTTPCode parse_requestline(char *temp, CheckState &checkState, HTTPRequest &request);
HTTPCode parse_headers(char *temp, check_state &checkState, HTTPRequest &request);
HTTPCode parse_body(char *temp, HTTPRequest &request, int checkIndex, int readIndex, Multipart &multipart);
HTTPCode parse_content(char *buff, int &check_index, CheckState &checkstate, int &readindex, int &start_line,
                       HTTPRequest &request, Multipart &multipart);
void parse_multipart(HTTPRequest &request);

std::string filePathFromCurDirGenerate(std::string fileName, std::string subDir = "",std::string executingPath="");

#endif  // SINGLE_THREAD_REQUEST_STATUS_MACHINE_H
