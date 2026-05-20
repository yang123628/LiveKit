#pragma once

#include <string>
#include <unordered_map>
#include <functional>

class HttpRequest;
class HttpResponse;

class Router {
public:
    using Handler = std::function<void(HttpRequest&, HttpResponse&)>;

    void get(const std::string& path, const Handler& handler);
    void post(const std::string& path, const Handler& handler);

    bool route(HttpRequest& req, HttpResponse& resp);

private:
    std::unordered_map<std::string, Handler> m_getRoutes;
    std::unordered_map<std::string, Handler> m_postRoutes;
};
