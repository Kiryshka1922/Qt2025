#pragma once
#include <QObject>
#include <QSslSocket>
#include <QImage>
#include <QUrl>
#include <QSslConfiguration>

class SslImageClient : public QObject {
    Q_OBJECT
public:
    explicit SslImageClient(QObject *parent = nullptr);
    void fetchImage(const QString &url);
    void setupGostSupport(); // Настройка поддержки ГОСТ алгоритмов

signals:
    void imageReceived(const QImage &image);
    void errorOccurred(const QString &error);

private slots:
    void onEncrypted();
    void onSslErrors(const QList<QSslError> &errors);
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onDisconnected();

private:
    void parseHttpResponse();
    QString getHeaderValue(const QStringList &headers, const QString &key) const;
    int getStatusCode(const QString &statusLine) const;
    QSslSocket *socket;
    QByteArray responseBuffer;
    bool headersReceived;
    int contentLength;
    QByteArray imageData;
    QString host;
    quint16 port;
    QString path;
    QSslConfiguration sslConfig;
    bool errorEmitted = false; // Флаг для предотвращения множественных сигналов ошибки
};

