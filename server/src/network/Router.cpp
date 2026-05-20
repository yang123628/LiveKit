#include "network/Router.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"

void Router::get(const std::string& path, const Handler& handler) {
    m_getRoutes[path] = handler;
}

void Router::post(const std::string& path, const Handler& handler) {
    m_postRoutes[path] = handler;
}

bool Router::route(HttpRequest& req, HttpResponse& resp) {
    const std::string& path = req.path();
    Handler handler;

    if (req.method() == HttpRequest::GET) {
        auto it = m_getRoutes.find(path);
        if (it != m_getRoutes.end()) {
            handler = it->second;
        }
    } else if (req.method() == HttpRequest::POST) {
        auto it = m_postRoutes.find(path);
        if (it != m_postRoutes.end()) {
            handler = it->second;
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
