#pragma once
#include <QSslServer>
#include <QSslSocket>
#include <QObject>
#include <QString>
#include <QMap>
#include <memory>
#include <vector>

class HttpRequestHandler;

// SSL сервер с поддержкой ГОСТ шифрования
class SslImageServer : public QSslServer {
    Q_OBJECT
public:
    explicit SslImageServer(QObject *parent = nullptr);
    ~SslImageServer();
    bool startServer(quint16 port = 8443);
    void setImageDirectory(const QString &dir);
    QString getImageDirectory() const { return imageDirectory; }
    
    // Настройка SSL сертификатов
    void setSslConfiguration(const QSslConfiguration &config);
    void setupGostSupport(); // Настройка поддержки ГОСТ алгоритмов

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void handleRequest();
    void onSslErrors(const QList<QSslError> &errors);
    void onEncrypted();
    void onDisconnected();

private:
    QString imageDirectory;
    QMap<QSslSocket*, QByteArray> requestBuffers;
    std::vector<std::unique_ptr<HttpRequestHandler>> handlers;
    QSslConfiguration sslConfig;
};

