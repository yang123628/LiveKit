#pragma once

#include <QJsonObject>
#include <QString>

class ApiResponse {
public:
    ApiResponse();
    explicit ApiResponse(const QJsonObject& json);

    int code() const;
    QString msg() const;
    QJsonObject data() const;

    bool isSuccess() const;
    QString errorMessage() const;

private:
    int m_code;
    QString m_msg;
    QJsonObject m_data;
};
