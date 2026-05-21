#pragma once

#include "network/IHttpClient.h"
#include <QTimer>
#include <QJsonArray>

class MockHttpClient : public IHttpClient {
    Q_OBJECT

public:
    explicit MockHttpClient(QObject* parent = nullptr);
    ~MockHttpClient() override = default;

    void get(const QString& path, const Callback& callback) override;
    void post(const QString& path, const QJsonObject& body, const Callback& callback) override;

private:
    void handleGetAvatars(const Callback& callback);
    void handleGetRooms(const QString& path, const Callback& callback);
    void handleLogin(const QJsonObject& body, const Callback& callback);
    void handleRegister(const QJsonObject& body, const Callback& callback);

    int m_nextUserId;
};
