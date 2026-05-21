#include "network/HttpRequest.h"
#include <sstream>

HttpRequest::HttpRequest()
    : m_method(UNKNOWN) {
}

bool HttpRequest::parse(const std::string& raw) {
    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) return false;

    std::string headerPart = raw.substr(0, headerEnd);
    m_body = raw.substr(headerEnd + 4);

    size_t firstLine = headerPart.find("\r\n");
    if (firstLine == std::string::npos) return false;

    parseRequestLine(headerPart.substr(0, firstLine));
    parseHeaders(headerPart.substr(firstLine + 2));

    return m_method != UNKNOWN;
}

HttpRequest::Method HttpRequest::method() const {
    return m_method;
}

const std::string& HttpRequest::path() const {
    return m_path;
}

const std::string& HttpRequest::version() const {
    return m_version;
}

const std::string& HttpRequest::body() const {
    return m_body;
}

std::string HttpRequest::getHeader(const std::string& key) const {
    auto it = m_headers.find(toLower(key));
    if (it != m_headers.end()) return it->second;
    return "";
}

std::string HttpRequest::getParam(const std::string& key) const {
    auto it = m_params.find(key);
    if (it != m_params.end()) return it->second;
    return "";
}

std::string HttpRequest::getPathParam(const std::string& key) const {
    auto it = m_pathParams.find(key);
    if (it != m_pathParams.end()) return it->second;
    return "";
}

nlohmann::json HttpRequest::getJson() const {
    try {
        if (m_body.empty()) return nlohmann::json::object();
        return nlohmann::json::parse(m_body);
    } catch (...) {
        return nlohmann::json::object();
    }
}

void HttpRequest::setPath(const std::string& path) {
    m_path = path;
}

void HttpRequest::setMethod(Method method) {
    m_method = method;
}

void HttpRequest::setPathParam(const std::string& key, const std::string& value) {
    m_pathParams[key] = value;
}

void HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream iss(line);
    std::string methodStr, fullPath, version;
    iss >> methodStr >> fullPath >> version;

    if (methodStr == "GET") m_method = GET;
    else if (methodStr == "POST") m_method = POST;
    else m_method = UNKNOWN;

    m_version = version;

    size_t queryPos = fullPath.find('?');
    if (queryPos != std::string::npos) {
        m_path = fullPath.substr(0, queryPos);
        parseQueryParams(fullPath.substr(queryPos + 1));
    } else {
        m_path = fullPath;
    }
}

void HttpRequest::parseHeaders(const std::string& headers) {
    std::istringstream iss(headers);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = toLower(line.substr(0, colon));
            std::string value = line.substr(colon + 1);
            while (!value.empty() && value[0] == ' ') {
                value.erase(0, 1);
            }
            m_headers[key] = value;
        }
    }
}

void HttpRequest::parseQueryParams(const std::string& query) {
    std::istringstream iss(query);
    std::string param;
    while (std::getline(iss, param, '&')) {
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            m_params[param.substr(0, eq)] = param.substr(eq + 1);
        }
    }
}

std::string HttpRequest::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
