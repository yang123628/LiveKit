#include "network/WebSocketClient.h"
#include "app/AppConfig.h"
#include <QDebug>

WebSocketClient::WebSocketClient(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_reconnectTimer(nullptr)
    , m_roomId(0)
    , m_reconnectCount(0)
    , m_maxReconnectCount(5)
    , m_manualClose(false)
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::tryReconnect);
}

WebSocketClient::~WebSocketClient() {
    disconnectFromServer();
}

void WebSocketClient::connectToServer(const QString& token, int roomId) {
    m_token = token;
    m_roomId = roomId;
    m_manualClose = false;
    m_reconnectCount = 0;

    if (m_socket) {
        m_socket->deleteLater();
    }

    m_socket = new QWebSocket();

    connect(m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);

    QString serverAddr = AppConfig::instance().serverAddress();
    serverAddr.replace("http://", "");
    QString url = QString("ws://%1:8080/ws?token=%2&room_id=%3")
        .arg(serverAddr)
        .arg(token)
        .arg(roomId);

    m_serverUrl = url;
    m_socket->open(QUrl(url));
}

void WebSocketClient::disconnectFromServer() {
    m_manualClose = true;
    m_reconnectTimer->stop();

    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

bool WebSocketClient::isConnected() const {
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::sendDanmaku(const QString& content) {
    if (!isConnected()) return;

    QJsonObject msg;
    msg["type"] = QString::fromUtf8("danmaku");
    msg["content"] = content;

    QJsonDocument doc(msg);
    m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void WebSocketClient::sendGift(int giftId) {
    if (!isConnected()) return;

    QJsonObject msg;
    msg["type"] = QString::fromUtf8("gift");
    msg["gift_id"] = giftId;

    QJsonDocument doc(msg);
    m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void WebSocketClient::sendLike() {
    if (!isConnected()) return;

    QJsonObject msg;
    msg["type"] = QString::fromUtf8("like");

    QJsonDocument doc(msg);
    m_socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) return;

    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();

    if (type == QString::fromUtf8("danmaku")) {
        QString username = obj.value("username").toString();
        QString content = obj.value("content").toString();
        emit danmakuReceived(username, content);
    } else if (type == QString::fromUtf8("gift")) {
        QString username = obj.value("username").toString();
        int giftId = obj.value("gift_id").toInt();
        QString giftName = obj.value("gift_name").toString();
        emit giftReceived(username, giftId, giftName);
    } else if (type == QString::fromUtf8("like")) {
        int count = obj.value("count").toInt();
        emit likeReceived(count);
    } else if (type == QString::fromUtf8("viewer_count")) {
        int count = obj.value("count").toInt();
        emit viewerCountChanged(count);
    } else if (type == QString::fromUtf8("viewer_join")) {
        QString username = obj.value("username").toString();
        emit viewerJoined(username);
    } else if (type == QString::fromUtf8("viewer_leave")) {
        QString username = obj.value("username").toString();
        emit viewerLeft(username);
    }
}

void WebSocketClient::onConnected() {
    m_reconnectCount = 0;
    emit connected();
}

void WebSocketClient::onDisconnected() {
    emit disconnected();

    if (!m_manualClose && m_reconnectCount < m_maxReconnectCount) {
        m_reconnectTimer->start(3000);
    }
}

void WebSocketClient::tryReconnect() {
    if (m_manualClose) return;

    m_reconnectCount++;
    qDebug() << "WebSocket reconnecting... attempt" << m_reconnectCount;

    if (m_socket) {
        m_socket->deleteLater();
    }

    m_socket = new QWebSocket();

    connect(m_socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);

    m_socket->open(QUrl(m_serverUrl));
}
