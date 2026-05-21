#include "network/Router.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include <sstream>

void Router::get(const std::string& path, const Handler& handler) {
    RouteEntry entry;
    entry.pattern = path;
    entry.handler = handler;

    std::istringstream iss(path);
    std::string segment;
    while (std::getline(iss, segment, '/')) {
        if (!segment.empty() && segment[0] == '{' && segment.back() == '}') {
            entry.paramNames.push_back(segment.substr(1, segment.size() - 2));
        }
    }

    m_getRoutes.push_back(entry);
}

void Router::post(const std::string& path, const Handler& handler) {
    RouteEntry entry;
    entry.pattern = path;
    entry.handler = handler;

    std::istringstream iss(path);
    std::string segment;
    while (std::getline(iss, segment, '/')) {
        if (!segment.empty() && segment[0] == '{' && segment.back() == '}') {
            entry.paramNames.push_back(segment.substr(1, segment.size() - 2));
        }
    }

    m_postRoutes.push_back(entry);
}

bool Router::matchRoute(const std::string& path, const RouteEntry& entry,
                         std::unordered_map<std::string, std::string>& outParams) {
    std::vector<std::string> pathSegments;
    std::vector<std::string> patternSegments;

    std::istringstream pathIss(path);
    std::string seg;
    while (std::getline(pathIss, seg, '/')) {
        if (!seg.empty()) pathSegments.push_back(seg);
    }

    std::istringstream patIss(entry.pattern);
    while (std::getline(patIss, seg, '/')) {
        if (!seg.empty()) patternSegments.push_back(seg);
    }

    if (pathSegments.size() != patternSegments.size()) return false;

    size_t paramIdx = 0;
    for (size_t i = 0; i < patternSegments.size(); ++i) {
        if (!patternSegments[i].empty() && patternSegments[i][0] == '{' && patternSegments[i].back() == '}') {
            if (paramIdx < entry.paramNames.size()) {
                outParams[entry.paramNames[paramIdx]] = pathSegments[i];
                ++paramIdx;
            }
        } else if (patternSegments[i] != pathSegments[i]) {
            return false;
        }
    }
    return true;
}

bool Router::route(HttpRequest& req, HttpResponse& resp) {
    const std::string& path = req.path();
    Handler handler;
    std::unordered_map<std::string, std::string> params;

    if (req.method() == HttpRequest::GET) {
        for (const auto& entry : m_getRoutes) {
            params.clear();
            if (matchRoute(path, entry, params)) {
                handler = entry.handler;
                for (const auto& p : params) {
                    req.setPathParam(p.first, p.second);
                }
                break;
            }
        }
    } else if (req.method() == HttpRequest::POST) {
        for (const auto& entry : m_postRoutes) {
            params.clear();
            if (matchRoute(path, entry, params)) {
                handler = entry.handler;
                for (const auto& p : params) {
                    req.setPathParam(p.first, p.second);
                }
                break;
            }
        }
    }

    if (handler) {
        handler(req, resp);
        return true;
    }

    resp.setStatus(404);
    resp.setJson(404, "Not Found");
    return false;
}
