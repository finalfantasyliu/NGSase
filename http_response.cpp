#include "http_response.h"

std::string HTTPResponse::responseToString(bool onlyHeader, bool expandHeader, bool bodyLengthCalculate, std::vector<std::string> expandHeaderContents) {
    std::string responseString;
    responseString += protocol + " " + status + "\r\n";
    if (responseType != NONE)
        responseString += "Content-Type: " + contentEnumToString(responseType) + "\r\n";

    if (!connection.empty())
        responseString += "Connection: " + connection + "\r\n";

    if (bodyLengthCalculate == true && filefd == 0) {
        responseString += "Content-Length: " + std::to_string(body.size());
    } else if (filefd > 0 && responseType != MULTIPART && responseType != BYTERANGE && bodyLengthCalculate == true) {
        struct stat stat_buf;
        fstat(filefd, &stat_buf);
        off_t len = stat_buf.st_size;
        contentLength = static_cast<int>(len);
        responseString += "Content-Length: " + std::to_string(contentLength);
        std::cout<<"file size:"<<contentLength<<std::endl;
    } else
        responseString += "Content-Length: " + std::to_string(contentLength);

    responseString += "\r\n";

    if (onlyHeader == false) {
        if (expandHeader == true && !expandHeaderContents.empty()) {
            for (auto &s : expandHeaderContents) {
                responseString += s;
                responseString += "\r\n";
            }
            responseString += "\r\n";
            responseString += body;
        } else {
            responseString += "\r\n";
            responseString += body;
        }
    } else if (onlyHeader == true) {
        if (expandHeader == false) {
            responseString += "\r\n";
        } else if (expandHeader == true && !expandHeaderContents.empty()) {
            for (auto &s : expandHeaderContents) {
                responseString += s;
                responseString += "\r\n";
            }
            responseString += "\r\n";
        }
    }

    return responseString;
}