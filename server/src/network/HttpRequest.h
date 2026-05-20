#pragma once

#include <string>
#include <unordered_map>
#include <algorithm>

class HttpRequest {
public:
    enum Method { GET, POST, UNKNOWN };

    HttpRequest();

    bool parse(const std::string& raw);

    Method method() const;
    const std::string& path() const;
    const std::string& version() const;
    const std::string& body() const;

    std::string getHeader(const std::string& key) const;
    std::string getParam(const std::string& key) const;

    void setPath(const std::string& path);
    void setMethod(Method method);

private:
    Method m_method;
    std::string m_path;
    std::string m_version;
    std::string m_body;
    std::unordered_map<std::string, std::string> m_headers;
    std::unordered_map<std::string, std::string> m_params;

    void parseRequestLine(const std::string& line);
    void parseHeaders(const std::string& headers);
    void parseQueryParams(const std::string& query);
    static std::string toLower(const std::string& str);
};
