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

    std::string content = readFile(filePath);
    if (content.empty()) {
        resp.setStatus(404);
        resp.setHeader("Content-Type", "application/json");
        resp.setBody("{\"code\":404,\"msg\":\"File Not Found\",\"data\":{}}");
        return;
    }

    std::string mime = getMimeType(filePath);
    resp.setStatus(200, "OK");
    resp.setHeader("Content-Type", mime);
    resp.setHeader("Content-Length", std::to_string(content.size()));
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
