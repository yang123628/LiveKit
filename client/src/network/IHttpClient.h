#pragma once

#include <QObject>
#include <QJsonObject>
#include <functional>
#include "network/ApiResponse.h"

class IHttpClient : public QObject {
    Q_OBJECT

public:
    using Callback = std::function<void(const ApiResponse&)>;

    explicit IHttpClient(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IHttpClient() = default;

    virtual void get(const QString& path, const Callback& callback) = 0;
    virtual void post(const QString& path, const QJsonObject& body, const Callback& callback) = 0;

    void setBaseUrl(const QString& url);
    QString baseUrl() const;

protected:
    QString m_baseUrl;
};
