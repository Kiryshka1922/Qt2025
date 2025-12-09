#include "ssl_image_server.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslCipher>
#include "../http/get_request_handler.h"

SslImageServer::SslImageServer(QObject *parent)
    : QSslServer(parent)
    , imageDirectory(QDir::currentPath())
{
    handlers.push_back(std::make_unique<GetRequestHandler>(static_cast<SslImageServer*>(this)));
    
    setupGostSupport();
}

SslImageServer::~SslImageServer() = default;

void SslImageServer::setupGostSupport() {
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    
    QFile certFile("server.crt");
    QFile keyFile("server.key");
    
    if (certFile.exists() && keyFile.exists()) {
        if (certFile.open(QIODevice::ReadOnly) && keyFile.open(QIODevice::ReadOnly)) {
            QSslCertificate cert(&certFile, QSsl::Pem);
            QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
            
            certFile.close();
            keyFile.close();
            
            if (!cert.isNull() && !key.isNull()) {
                config.setLocalCertificate(cert);
                config.setPrivateKey(key);
            }
        }
    }
    QString cipherString = "ECDHE-RSA-AES256-GCM-SHA384:"
                          "ECDHE-RSA-AES128-GCM-SHA256:"
                          "AES256-GCM-SHA384:"
                          "AES128-GCM-SHA256:"
                          "ECDHE-RSA-AES256-SHA384:"
                          "ECDHE-RSA-AES128-SHA256";
    
    config.setCiphers(cipherString);
    
    config.setProtocol(QSsl::TlsV1_2OrLater);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    
    sslConfig = config;
    setSslConfiguration(config);
}

bool SslImageServer::startServer(quint16 port) {
    return listen(QHostAddress::Any, port);
}

void SslImageServer::setImageDirectory(const QString &dir) {
    imageDirectory = dir;
    QDir d(dir);
    if (!d.exists()) {
        d.mkpath(".");
    }
}

void SslImageServer::setSslConfiguration(const QSslConfiguration &config) {
    sslConfig = config;
    QSslServer::setSslConfiguration(config);
}

void SslImageServer::incomingConnection(qintptr socketDescriptor) {
    QSslSocket *socket = new QSslSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    requestBuffers[socket] = QByteArray();
    
    // Настраиваем SSL на сокете
    socket->setSslConfiguration(sslConfig);
    
    connect(socket, &QSslSocket::encrypted, this, &SslImageServer::onEncrypted);
    connect(socket, &QSslSocket::sslErrors, this, &SslImageServer::onSslErrors);
    connect(socket, &QSslSocket::readyRead, this, &SslImageServer::handleRequest);
    connect(socket, &QSslSocket::disconnected, this, &SslImageServer::onDisconnected);
    connect(socket, &QSslSocket::disconnected, socket, &QSslSocket::deleteLater);
    
    // Начинаем SSL handshake
    socket->startServerEncryption();
}

void SslImageServer::onEncrypted() {
    // SSL соединение установлено
}

void SslImageServer::onSslErrors(const QList<QSslError> &errors) {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
    if (!socket) return;
    
    // Для тестирования игнорируем ошибки сертификатов
    Q_UNUSED(errors);
    socket->ignoreSslErrors();
}

void SslImageServer::handleRequest() {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
    if (!socket || !socket->isEncrypted()) {
        return; // Ждем завершения SSL handshake
    }

    requestBuffers[socket].append(socket->readAll());
    QByteArray &buffer = requestBuffers[socket];

    // Проверяем, есть ли полный HTTP запрос (заканчивается на \r\n\r\n)
    if (!buffer.contains("\r\n\r\n")) {
        return; // Ждем больше данных
    }

    QString request = QString::fromUtf8(buffer);
    QStringList lines = request.split("\r\n");
    
    if (lines.isEmpty()) {
        socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }

    QString requestLine = lines[0];
    QStringList parts = requestLine.split(" ");
    
    if (parts.size() < 3) {
        socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
        socket->disconnectFromHost();
        return;
    }

    QString method = parts[0];
    QString path = parts[1];

    bool handled = false;
    for (auto &handler : handlers) {
        if (handler->canHandle(method, path)) {
            handler->handleRequest(socket, method, path);
            handled = true;
            break;
        }
    }

    if (!handled) {
        socket->write("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
    }

    requestBuffers.remove(socket);
    socket->disconnectFromHost();
}

void SslImageServer::onDisconnected() {
    QSslSocket *socket = qobject_cast<QSslSocket*>(sender());
    if (socket) {
        requestBuffers.remove(socket);
    }
}

