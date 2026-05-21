#include "network/HttpClient.h"
#include <QNetworkRequest>
#include <QUrl>

HttpClient::HttpClient(QObject* parent)
    : IHttpClient(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

HttpClient::~HttpClient() = default;

void HttpClient::get(const QString& path, const Callback& callback) {
    QUrl url(m_baseUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto* reply = m_manager->get(request);
    setupReply(reply, callback);
}

void HttpClient::post(const QString& path, const QJsonObject& body, const Callback& callback) {
    QUrl url(m_baseUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(body);
    auto* reply = m_manager->post(request, doc.toJson());
    setupReply(reply, callback);
}

void HttpClient::setupReply(QNetworkReply* reply, const Callback& callback) {
    auto* timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, [reply, timer, callback]() {
        reply->abort();
        timer->deleteLater();
        QJsonObject respObj;
        respObj["code"] = -1;
        respObj["msg"] = QString::fromUtf8("请求超时");
        callback(ApiResponse(respObj));
    });

    connect(reply, &QNetworkReply::finished, [this, reply, timer, callback]() {
        timer->stop();
        timer->deleteLater();
        handleReply(reply, callback);
    });

    timer->start(TIMEOUT_MS);
}

void HttpClient::handleReply(QNetworkReply* reply, const Callback& callback) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QJsonObject respObj;
        respObj["code"] = -1;
        respObj["msg"] = reply->errorString();
        callback(ApiResponse(respObj));
        return;
    }

    auto data = reply->readAll();
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError) {
        QJsonObject respObj;
        respObj["code"] = -1;
        respObj["msg"] = QString::fromUtf8("响应解析失败");
        callback(ApiResponse(respObj));
        return;
    }

    ApiResponse resp(doc.object());
    callback(resp);
}
