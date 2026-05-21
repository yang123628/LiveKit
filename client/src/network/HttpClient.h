#pragma once

#include "network/IHttpClient.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>

class HttpClient : public IHttpClient {
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);
    ~HttpClient() override;

    void get(const QString& path, const Callback& callback) override;
    void post(const QString& path, const QJsonObject& body, const Callback& callback) override;

private:
    void setupReply(QNetworkReply* reply, const Callback& callback);
    void handleReply(QNetworkReply* reply, const Callback& callback);

    QNetworkAccessManager* m_manager;
    static const int TIMEOUT_MS = 10000;
};
