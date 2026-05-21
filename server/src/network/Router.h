#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

class HttpRequest;
class HttpResponse;

struct RouteEntry {
    std::string pattern;
    std::vector<std::string> paramNames;
    std::function<void(HttpRequest&, HttpResponse&)> handler;
};

class Router {
public:
    using Handler = std::function<void(HttpRequest&, HttpResponse&)>;

    void get(const std::string& path, const Handler& handler);
    void post(const std::string& path, const Handler& handler);

    bool route(HttpRequest& req, HttpResponse& resp);

private:
    bool matchRoute(const std::string& path, const RouteEntry& entry,
                    std::unordered_map<std::string, std::string>& outParams);
    std::vector<RouteEntry> m_getRoutes;
    std::vector<RouteEntry> m_postRoutes;
};
