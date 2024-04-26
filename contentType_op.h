#ifndef NGSASE_CONTENTTYPE_OP_H
#define NGSASE_CONTENTTYPE_OP_H
#include <string>
#include <iostream>
enum ContentType {
    JSON,
    JAVASCRIPT,
    FORMS,
    MULTIPART,
    TEXT,
    CSS,
    HTML,
    XML,
    IMAGE,
    IMAGE_JPEG,
    IMAGE_PNG,
    IMAGE_GIF,
    IMAGE_SVG,
    IMAGE_ICO,
    BINARY,
    BYTERANGE,
    NONE

};
ContentType contentTypeToEnum(std::string &type);
std::string contentEnumToString(ContentType &enumType);

#endif