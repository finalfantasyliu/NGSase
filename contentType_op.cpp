#include "contentType_op.h"

ContentType contentTypeToEnum(std::string &type) {
    if (type.find("multipart/form-data") != std::string::npos)
        return MULTIPART;
    else if (type.find("application/json") != std::string::npos)
        return JSON;
    else if (type.find("application/javascript") != std::string::npos)
        return JAVASCRIPT;
    else if (type.find("application/x-www-form-urlencoded") != std::string::npos)
        return FORMS;
    else if (type.find("text/plain") != std::string::npos)
        return TEXT;
    else if (type.find("text/css") != std::string::npos)
        return CSS;
    else if (type.find("text/html") != std::string::npos)
        return HTML;
    else if (type.find("application/xml") != std::string::npos)
        return XML;
    else if (type.find("image") != std::string::npos)
        return IMAGE;
    else if (type.find("image/jpeg") != std::string::npos)
        return IMAGE_JPEG;
    else if (type.find("image/png") != std::string::npos)
        return IMAGE_PNG;
    else if (type.find("image/gif") != std::string::npos)
        return IMAGE_GIF;
    else if (type.find("image/svg+xml") != std::string::npos)
        return IMAGE_SVG;
    else if (type.find("image/x-icon") != std::string::npos)
        return IMAGE_ICO;
    else if (type.find("application/octet-stream") != std::string::npos)
        return BINARY;
    else if (type.find("multipart/byteranges") != std::string::npos)
        return BYTERANGE;
    else
        return NONE;
}

std::string contentEnumToString(ContentType &enumType) {
    switch (enumType) {
        case MULTIPART: {
            return "multipart/form-data";
            break;
        }
        case JSON: {
            return "application/json";
            break;
        }
        case JAVASCRIPT: {
            return "application/javascript";
            break;
        }
        case FORMS: {
            return "application/x-www-form-urlencoded";
            break;
        }
        case TEXT: {
            return "text/plain";
            break;
        }
        case CSS: {
            return "text/css";
            break;
        }
        case HTML: {
            return "text/html";
            break;
        }
        case XML: {
            return "application/xml";
            break;
        }
        case IMAGE: {
            return "image";
            break;
        }
        case IMAGE_JPEG: {
            return "image/jpeg";
            break;
        }
        case IMAGE_PNG: {
            return "image/png";
            break;
        }
        case IMAGE_GIF: {
            return "image/gif";
            break;
        }
        case IMAGE_SVG: {
            return "image/svg+xml";
            break;
        }
        case IMAGE_ICO: {
            return "image/x-icon";
            break;
        }
        case BINARY: {
            return "application/octet-stream";
            break;
        }
        case BYTERANGE: {
            return "multipart/byteranges";
            break;
        }
        default: {
            std::cout << "Undefined behaviour" << std::endl;
            return "";
        }
    }
}