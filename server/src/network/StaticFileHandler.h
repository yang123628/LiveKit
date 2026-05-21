#pragma once

#include <string>

class HttpRequest;
class HttpResponse;

class StaticFileHandler {
public:
    static void handle(HttpRequest& req, HttpResponse& resp);

private:
    static std::string getMimeType(const std::string& path);
    static std::string readFile(const std::string& path);
    static std::string readFileRange(const std::string& path, size_t offset, size_t length);
    static size_t getFileSize(const std::string& path);
};
