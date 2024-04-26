//
// Created by 劉軒豪 on 2024/1/31.
//
// 可以寫的static 的變數讓使用者指定path
#include "http_request.h"
std::string filePathFromCurDirGenerate(std::string fileName, std::string subDir, std::string executingPath) {
    std::string finalPath;
    std::filesystem::path currentPath;
    std::filesystem::path executablePath = executingPath;
    if (!executablePath.empty())
        currentPath = executablePath.parent_path();
    else
        currentPath = std::filesystem::current_path();
    std::filesystem::path combinePathWithoutSubDir = currentPath / fileName;

    if (subDir.empty()) {
        try {
            while (std::filesystem::exists(combinePathWithoutSubDir)) {
                std::string baseName = combinePathWithoutSubDir.stem().string();

                if (baseName.find('.') != std::string::npos) {
                    size_t dotPos = baseName.find('.');
                    std::string baseNameWithoutDot = baseName.substr(0, dotPos);
                    std::string newFileName = baseNameWithoutDot + "_copy";
                    fileName = fileName.replace(0, baseNameWithoutDot.length(), newFileName);
                } else {
                    std::string newFileName = baseName + "_copy";
                    fileName = fileName.replace(0, baseName.length(), newFileName);
                }
                combinePathWithoutSubDir = currentPath / fileName;
            }
        } catch (const std::exception &e) {
            finalPath = "Error:" + std::string(e.what());
            return finalPath;
        }

        finalPath = combinePathWithoutSubDir.string();

        return finalPath;
    }
    std::filesystem::path subDirPath = currentPath / subDir;
    try {
        if (!std::filesystem::exists(subDirPath) || !std::filesystem::is_directory(subDirPath))
            std::filesystem::create_directories(subDirPath);
        std::filesystem::path checkFileExisted = subDirPath / fileName;
       /* while (std::filesystem::exists(checkFileExisted)) {
            std::string baseName = checkFileExisted.stem().string();

            if (baseName.find('.') != std::string::npos) {
                size_t dotPos = baseName.find('.');
                std::string baseNameWithoutDot = baseName.substr(0, dotPos);
                std::string newFileName = baseNameWithoutDot + "_copy";
                fileName = fileName.replace(0, baseNameWithoutDot.length(), newFileName);
            } else {
                std::string newFileName = baseName + "_copy";
                fileName = fileName.replace(0, baseName.length(), newFileName);
            }
            checkFileExisted = subDirPath / fileName;
        }*/
        finalPath = checkFileExisted.string();
    } catch (const std::exception &e) {
        finalPath = "Error: " + std::string(e.what());
    }

    return finalPath;
};




LineState parse_line(char *buffer, int &check_index, int &read_index) {
    char temp;
    for (; check_index < read_index; check_index++) {
        temp = buffer[check_index];
        if (temp == '\r') {
            if ((check_index + 1) == read_index) {
                return LINE_OPEN;
            } else if (buffer[check_index + 1] == '\n') {
                buffer[check_index++] = '\0';
                buffer[check_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (temp == '\n') {
            if ((check_index > 1) && buffer[check_index - 1] == '\r') {
                buffer[check_index - 1] = '\0';
                buffer[check_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

HTTPCode parse_requestline(char *temp, CheckState &checkState, HTTPRequest &request) {
    char *url = strpbrk(temp, " \t");
    if (!url) {
        return BAD_REQUEST;
    }
    *url++ = '\0';
    char *method = temp;
    if (strcasecmp(method, "GET") == 0) {
        printf("The request method is GET\n");
        request.method = Method::GET;

    } else if (strcasecmp(method, "POST") == 0) {
        printf("The request method is POST\n");
        request.method = Method::POST;
    } else {
        return BAD_REQUEST;
    }
    url += strspn(url, " \t");
    char *version = strpbrk(url, " \t");
    if (!version) {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    *version += strspn(version, " \t");
    if (strcasecmp(version, "HTTP/1.1") != 0) {
        return BAD_REQUEST;
    } else {
        request.version = version;
    }

    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    }

    if (!url || url[0] != '/') {
        return BAD_REQUEST;
    } else {
        request.URL = url;
    }
    printf("The request URL is:%s\n", url);
    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTPCode parse_headers(char *temp, check_state &checkState, HTTPRequest &request) {
    char *nullCharPtr = strchr(temp, '\0');
    if (temp[0] == '\0' && temp[1] == '\0') {
        checkState = CHECK_STATE_BODY;
        request.headerComplete = true;
        return GET_HEADER;

    } else if (strncasecmp(temp, "Host:", 5) == 0) {
        temp += 5;
        temp += strspn(temp, " \t");
        printf("this request host is:%s\n", temp);
        request.Host = temp;

    } else if (strncasecmp(temp, "Content-Type: ", 14) == 0) {
        temp += 14;
        std::cout << "Content-Type:" << temp << std::endl;
        std::string contentType(temp);
        ContentType type = contentTypeToEnum(contentType);
        request.contentType = type;
    } else if (strncasecmp(temp, "Content-Length: ", 16) == 0) {
        temp += 16;
        std::cout << "Content-Length:" << temp << std::endl;
        request.contentLength = atoi(temp);
    } else if (strncasecmp(temp, "Connection: ", 12) == 0) {
        temp += 12;
        std::cout << "Connection:" << temp << std::endl;
        request.connection = temp;
    } else
        request.restHeaders.push_back(temp);

    return NO_REQUEST;
}

HTTPCode parse_body(char *temp, HTTPRequest &request, int checkIndex, int readIndex, Multipart &multipart) {
    if (request.contentLength == 0) {
        return GET_REQUEST;
    }

    if (request.contentLength > 0 && request.contentReadNumber < request.contentLength) {
        int remain = readIndex - checkIndex;
        std::string body(temp, remain);
        switch (request.contentType) {
            case BINARY: {
                if (request.outputFile.is_open()) {
                    // 使用try-catch確認write()funtion使否有出錯，若出錯一樣也交給後續處理，回傳錯誤訊息給client端
                    try {
                        request.outputFile.write(temp, remain);
                        if (request.outputFile.fail()) {
                            std::filesystem::remove(request.fileLocation);
                            throw std::runtime_error("writting failed");
                        }
                        request.contentReadNumber += remain;
                    } catch (const std::exception &e) {
                        request.body = "Error: " + std::string(e.what());
                        // 若錯誤要把fd 關閉
                        if (request.outputFile.is_open())
                            request.outputFile.close();
                        return GET_REQUEST;
                    }

                } else {
                    // 一般application/octet-stream請求是無法將file name的資料是不包含file name的相關file 訊息，
                    // 除非使用multipart或是將file name寫到URL裡，因multipart尚未編寫完成，乃採用特製header去存取相關訊息
                    // 之後如果要將reactor做成通用版在詳細的將其補全
                    std::string fileName;
                    std::string subDir ="data";
                    std::string email;
                    std::string filePath;
                    std::string pipelineName;
                    std::string projectName;
                    // 確認header內是不是有特殊header，若含有fileName
                    for (int i = 0; i < request.restHeaders.size(); i++) {
                        if (request.restHeaders[i].find("fileName") != std::string::npos) {
                            fileName = request.restHeaders[i].substr(10);
                        } else if (request.restHeaders[i].find("email") != std::string::npos) {
                            email = request.restHeaders[i].substr(7);
                        } else if (request.restHeaders[i].find("pipelineName") != std::string::npos)
                            pipelineName = request.restHeaders[i].substr(14);
                        else if (request.restHeaders[i].find("projectName") != std::string::npos)
                            projectName = request.restHeaders[i].substr(13);
                    }
                    // 如果這邊儲存到統一資料夾，當多個使用者添加相同的資料會有data race的情形。
                    if (request.URL.find("pipeline") != std::string::npos) {
                        subDir = subDir +"/ngsase_data" +"/pipeline_rawdata/" + email + "/" + pipelineName;
                    } else if (request.URL.find("project") != std::string::npos) {
                        subDir = subDir + "/ngsase_data" + "/project_rawdata/" + email + "/" + projectName;
                    }

                    // 產生相對應資料夾路徑
                    filePath = filePathFromCurDirGenerate(fileName, subDir);
                    // 如果創造路徑出現問題，代表filesystem create directories有一些問題，
                    // 若有將body設為錯誤訊息後，將錯誤訊息經由後續processTask處理後，顯示給client端
                    if (filePath.find("Error: ") != std::string::npos) {
                        std::cout << "subDir creation failed" << std::endl;
                        std::cout << filePath << std::endl;
                        request.body = filePath;

                        return GET_REQUEST;
                    }
                    try {
                        request.fileLocation = filePath;
                        request.outputFile.open(request.fileLocation, std::ios::binary);
                        // 因為是write file初始化所以需要確認is_open()
                        if (!request.outputFile.is_open())
                            throw std::runtime_error("Failed open file");
                        request.outputFile.write(temp, remain);
                        if (request.outputFile.fail())
                            throw std::runtime_error("writting failed");
                        request.contentReadNumber += remain;
                    } catch (const std::exception &e) {
                        request.body = "Error: " + std::string(e.what());
                        std::cout << "write file or open file failed" << std::endl;
                        std::cout << request.body << std::endl;
                        // 若錯誤要把fd關閉
                        if (request.outputFile.is_open())
                            request.outputFile.close();
                        return GET_REQUEST;
                    }
                }
                return NO_REQUEST;
                break;
            }
            case TEXT: {
                request.body += body;
                request.contentReadNumber += remain;

                return NO_REQUEST;
                break;
            }
            case JSON: {
                request.body += body;
                request.contentReadNumber += remain;

                return NO_REQUEST;
                break;
            }
            case MULTIPART: {
                if (request.contentLength > 25000000)
                    return LONG_REQUEST;

                request.body += body;
                request.contentReadNumber += remain;

                return NO_REQUEST;
                break;
            }
            default: {
                return BAD_REQUEST;
            }
        }
    }

    return GET_REQUEST;
}

HTTPCode parse_content(char *buff, int &check_index, CheckState &checkstate, int &readindex, int &start_line,
                       HTTPRequest &request, Multipart &multipart) {
    LineState lineState = LINE_OK;
    HTTPCode retcode = NO_REQUEST;

    while (1) {
        char *temp = nullptr;
        if (!request.headerComplete) {
            lineState = parse_line(buff, check_index, readindex);
            if (lineState == LINE_BAD) {
                return BAD_REQUEST;
                break;
            }
            temp = buff + start_line;
            start_line = check_index;
        } else {
            temp = buff + start_line;
            start_line = check_index;
        }

        switch (checkstate) {
            case CHECK_STATE_REQUESTLINE: {
                retcode = parse_requestline(temp, checkstate, request);
                if (retcode == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            case CHECK_STATE_HEADER: {
                retcode = parse_headers(temp, checkstate, request);
                if (retcode == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (retcode == GET_HEADER) {
                    if (check_index < readindex) {
                        checkstate = CHECK_STATE_BODY;
                        continue;
                    } else
                        return GET_HEADER;
                }

                break;
            }
            case CHECK_STATE_BODY: {
                retcode = parse_body(temp, request, check_index, readindex, multipart);
                if (retcode == GET_REQUEST)
                    return GET_REQUEST;
                else if (retcode == NO_REQUEST)
                    return NO_REQUEST;
            }

            default:
                return INTERNAL_ERROR;
        }
    }
    if (lineState == LINE_OPEN) {
        return NO_REQUEST;

    } else
        return BAD_REQUEST;
}

// 尚未測試
void parse_multipart(HTTPRequest &request) {
    std::string temp = request.body;
    request.body.clear();
    std::string boundary = request.boundary;
    int startIndex = 0;
    int boundarySize = boundary.size();

    while (temp.find(boundary, startIndex) != std::string::npos) {
        startIndex += boundarySize;
        if (temp[startIndex] == '-')
            break;
        startIndex += 2;
        Multipart portion;
        while (1) {
            int nextLine = temp.find("\r\n", startIndex);
            std::string line = temp.substr(startIndex, nextLine - startIndex);

            if (line.empty()) {
                startIndex = nextLine + 2;
                break;
            } else if (line.find("Content-Disposition") != std::string::npos) {
                int dispositionStart = line.find(":") + 2;
                portion.Disposition = line.substr(dispositionStart, nextLine - dispositionStart);
            } else if (line.find("Content-Type:") != std::string::npos) {
                int typeStart = line.find(":") + 2;
                std::string type = line.substr(typeStart, nextLine - typeStart);
                ContentType determineType = contentTypeToEnum(type);
                portion.type = determineType;
            }
            startIndex = nextLine + 2;
        }

        int nextLine = temp.find("\r\n", startIndex);
        portion.content = temp.substr(startIndex, nextLine - startIndex);
        startIndex = nextLine + 2;

        if (portion.type == BINARY) {
            request.outputFile.open("mysql_multi.csv.gz", std::ios::binary);
            request.outputFile.write(portion.content.c_str(), portion.content.size());
            request.outputFile.flush();
            request.outputFile.close();
        }
        request.multipart.push_back(portion);
    }
}
