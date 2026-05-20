#include "network/HttpResponse.h"

HttpResponse::HttpResponse()
    : m_statusCode(200),
      m_statusMessage("OK") {
}

void HttpResponse::setStatus(int code, const std::string& message) {
    m_statusCode = code;
    if (message.empty()) {
        switch (code) {
            case 200: m_statusMessage = "OK"; break;
            case 400: m_statusMessage = "Bad Request"; break;
            case 401: m_statusMessage = "Unauthorized"; break;
            case 404: m_statusMessage = "Not Found"; break;
            case 405: m_statusMessage = "Method Not Allowed"; break;
            case 500: m_statusMessage = "Internal Server Error"; break;
            default: m_statusMessage = "Unknown"; break;
        }
    } else {
        m_statusMessage = message;
    }
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    m_headers[key] = value;
}

void HttpResponse::setBody(const std::string& body) {
    m_body = body;
    m_headers["Content-Length"] = std::to_string(m_body.size());
}

void HttpResponse::setJson(int code, const std::string& msg, const std::string& data) {
    m_statusCode = code;
    m_statusMessage = (code == 200 || code == 0) ? "OK" : "Error";
    if (code == 0) m_statusCode = 200;
    m_headers["Content-Type"] = "application/json";
    std::string json = "{\"code\":" + std::to_string(code) + ",\"msg\":\"" + msg + "\",\"data\":" + data + "}";
    m_body = json;
    m_headers["Content-Length"] = std::to_string(m_body.size());
}

std::string HttpResponse::serialize() const {
    std::string result = "HTTP/1.1 " + std::to_string(m_statusCode) + " " + m_statusMessage + "\r\n";
    for (const auto& header : m_headers) {
        result += header.first + ": " + header.second + "\r\n";
    }
    result += "\r\n";
    result += m_body;
    return result;
}

int HttpResponse::statusCode() const {
    return m_statusCode;
}
