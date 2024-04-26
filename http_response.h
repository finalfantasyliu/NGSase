#ifndef NGSASE_HTTP_RESPONSE
#define NGSASE_HTTP_RESPONSE
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "contentType_op.h"

class HTTPResponse {
   public:
    HTTPResponse(){};

    HTTPResponse(std::string protocol,
                 std::string status,
                 ContentType type,
                 int contentLength,
                 std::string connection,
                 std::string body,
                 int filefd) : responseType{type}, contentLength{contentLength}, protocol{protocol}, status{status}, connection{connection}, body{body}, filefd{filefd} {};

    HTTPResponse(std::string protocol,
                 std::string status,
                 ContentType type,
                 std::string body,
                 std::string connection) : protocol{protocol}, status{status}, responseType{type}, body{body}, connection{connection} {};

   public:
    ContentType responseType = NONE;
    int contentLength = 0;
    std::string status;
    std::string protocol;
    std::string connection;
    std::string body;
    // multipart-response之後在支援

    int filefd = 0;

   public:
    std::string responseToString(bool onlyHeader, bool expandHeader, bool bodyLengthCalculate, std::vector<std::string> expandHeaderContents={}); 
};
#endif