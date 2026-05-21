#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(QObject* parent = nullptr);
    ~WebSocketClient();

    void connectToServer(const QString& token, int roomId);
    void disconnectFromServer();
    bool isConnected() const;

    void sendDanmaku(const QString& content);
    void sendGift(int giftId);
    void sendLike();

signals:
    void danmakuReceived(const QString& username, const QString& content);
    void giftReceived(const QString& username, int giftId, const QString& giftName);
    void likeReceived(int count);
    void viewerCountChanged(int count);
    void viewerJoined(const QString& username);
    void viewerLeft(const QString& username);
    void connected();
    void disconnected();
    void error(const QString& msg);

private:
    void onTextMessageReceived(const QString& message);
    void onConnected();
    void onDisconnected();
    void tryReconnect();

    QWebSocket* m_socket;
    QTimer* m_reconnectTimer;
    QString m_token;
    int m_roomId;
    int m_reconnectCount;
    int m_maxReconnectCount;
    bool m_manualClose;
    QString m_serverUrl;
};
