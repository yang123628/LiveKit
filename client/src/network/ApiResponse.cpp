#include "network/ApiResponse.h"

ApiResponse::ApiResponse()
    : m_code(-1)
    , m_msg("")
{
}

ApiResponse::ApiResponse(const QJsonObject& json)
    : m_code(json.value("code").toInt(-1))
    , m_msg(json.value("msg").toString(""))
    , m_data(json.value("data").toObject())
{
}

int ApiResponse::code() const {
    return m_code;
}

QString ApiResponse::msg() const {
    return m_msg;
}

QJsonObject ApiResponse::data() const {
    return m_data;
}

bool ApiResponse::isSuccess() const {
    return m_code == 0;
}

QString ApiResponse::errorMessage() const {
    if (isSuccess()) return "";
    return m_msg.isEmpty() ? QString("请求失败 (code: %1)").arg(m_code) : m_msg;
}
