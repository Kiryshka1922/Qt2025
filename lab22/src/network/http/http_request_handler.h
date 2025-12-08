#pragma once
#include <QTcpSocket>
#include <QString>

// Strategy Pattern для обработки HTTP запросов
class HttpRequestHandler {
public:
    virtual ~HttpRequestHandler() = default;
    virtual bool canHandle(const QString &method, const QString &path) = 0;
    virtual void handleRequest(QTcpSocket *socket, const QString &method, const QString &path) = 0;
};

