#include "network/IHttpClient.h"

void IHttpClient::setBaseUrl(const QString& url) {
    m_baseUrl = url;
}

QString IHttpClient::baseUrl() const {
    return m_baseUrl;
}
