#pragma once
#include "http_request_handler.h"
#include <QString>
#include <QTcpSocket>

class SslImageServer;

// Конкретная стратегия для обработки GET запросов
class GetRequestHandler : public HttpRequestHandler {
public:
    explicit GetRequestHandler(SslImageServer *server);
    bool canHandle(const QString &method, const QString &path) override;
    void handleRequest(QTcpSocket *socket, const QString &method, const QString &path) override;

private:
    SslImageServer *sslServer;
    QString getImageDirectory() const;
    void sendImageResponse(QTcpSocket *socket, const QString &filePath);
    void sendDirectoryListing(QTcpSocket *socket);
    void sendErrorResponse(QTcpSocket *socket, int code, const QString &message);
};

