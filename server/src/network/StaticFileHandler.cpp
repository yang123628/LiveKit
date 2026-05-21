#include "network/StaticFileHandler.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "core/Logger.h"
#include <fstream>
#include <sstream>

void StaticFileHandler::handle(HttpRequest& req, HttpResponse& resp) {
    std::string path = req.path();

    std::string filePath;
    if (path.find("/avatars/") == 0) {
        filePath = "static/avatars/" + path.substr(9);
    } else if (path.find("/gifts/") == 0) {
        filePath = "static/gifts/" + path.substr(7);
    } else if (path.find("/covers/") == 0) {
        filePath = "static/covers/" + path.substr(8);
    } else if (path.find("/recordings/") == 0) {
        filePath = "static/recordings/" + path.substr(12);
    } else {
        resp.setStatus(404);
        resp.setHeader("Content-Type", "application/json");
        resp.setBody("{\"code\":404,\"msg\":\"Not Found\",\"data\":{}}");
        return;
    }

    if (filePath.find("..") != std::string::npos) {
        resp.setStatus(403);
        resp.setHeader("Content-Type", "application/json");
        resp.setBody("{\"code\":403,\"msg\":\"Forbidden\",\"data\":{}}");
        return;
    }

    size_t fileSize = getFileSize(filePath);
    if (fileSize == 0) {
        resp.setStatus(404);
        resp.setHeader("Content-Type", "application/json");
        resp.setBody("{\"code\":404,\"msg\":\"File Not Found\",\"data\":{}}");
        return;
    }

    std::string mime = getMimeType(filePath);
    std::string rangeHeader = req.getHeader("Range");

    if (!rangeHeader.empty() && rangeHeader.find("bytes=") == 0) {
        std::string rangeSpec = rangeHeader.substr(6);
        size_t dashPos = rangeSpec.find('-');
        if (dashPos != std::string::npos) {
            size_t rangeStart = 0;
            size_t rangeEnd = fileSize - 1;

            std::string startStr = rangeSpec.substr(0, dashPos);
            std::string endStr = rangeSpec.substr(dashPos + 1);

            if (!startStr.empty()) {
                rangeStart = std::stoull(startStr);
            }
            if (!endStr.empty()) {
                rangeEnd = std::stoull(endStr);
            }

            if (rangeStart >= fileSize) {
                resp.setStatus(416);
                resp.setHeader("Content-Range", "bytes */" + std::to_string(fileSize));
                return;
            }

            if (rangeEnd >= fileSize) {
                rangeEnd = fileSize - 1;
            }

            size_t contentLength = rangeEnd - rangeStart + 1;
            std::string content = readFileRange(filePath, rangeStart, contentLength);

            resp.setStatus(206, "Partial Content");
            resp.setHeader("Content-Type", mime);
            resp.setHeader("Content-Length", std::to_string(contentLength));
            resp.setHeader("Content-Range", "bytes " + std::to_string(rangeStart) + "-" + std::to_string(rangeEnd) + "/" + std::to_string(fileSize));
            resp.setHeader("Accept-Ranges", "bytes");
            resp.setHeader("Access-Control-Allow-Origin", "*");
            resp.setBody(content);
            return;
        }
    }

    std::string content = readFile(filePath);
    resp.setStatus(200, "OK");
    resp.setHeader("Content-Type", mime);
    resp.setHeader("Content-Length", std::to_string(fileSize));
    resp.setHeader("Accept-Ranges", "bytes");
    resp.setHeader("Cache-Control", "max-age=3600");
    resp.setHeader("Access-Control-Allow-Origin", "*");
    resp.setBody(content);
}

std::string StaticFileHandler::getMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot + 1);

    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "ico") return "image/x-icon";
    if (ext == "mp4") return "video/mp4";
    if (ext == "webm") return "video/webm";
    if (ext == "html") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";

    return "application/octet-stream";
}

std::string StaticFileHandler::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string StaticFileHandler::readFileRange(const std::string& path, size_t offset, size_t length) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";
    file.seekg(offset);
    std::string content(length, '\0');
    file.read(&content[0], length);
    content.resize(file.gcount());
    return content;
}

size_t StaticFileHandler::getFileSize(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    return static_cast<size_t>(file.tellg());
}
