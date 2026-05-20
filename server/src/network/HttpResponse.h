#pragma once

#include <string>
#include <unordered_map>

class HttpResponse {
public:
    HttpResponse();

    void setStatus(int code, const std::string& message = "");
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

    void setJson(int code, const std::string& msg, const std::string& data = "{}");

    std::string serialize() const;

    int statusCode() const;

private:
    int m_statusCode;
    std::string m_statusMessage;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
};
