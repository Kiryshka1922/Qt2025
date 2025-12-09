#include "ssl_image_client.h"
#include <QDebug>
#include <QUrl>
#include <QSslConfiguration>
#include <QSslCipher>
#include <QSslSocket>

SslImageClient::SslImageClient(QObject *parent)
    : QObject(parent), socket(nullptr), headersReceived(false), contentLength(0)
{
    setupGostSupport();
}

void SslImageClient::setupGostSupport()
{
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();

    QString cipherString = "ECDHE-RSA-AES256-GCM-SHA384:"
                           "ECDHE-RSA-AES128-GCM-SHA256:"
                           "AES256-GCM-SHA384:"
                           "AES128-GCM-SHA256:"
                           "ECDHE-RSA-AES256-SHA384:"
                           "ECDHE-RSA-AES128-SHA256";

    config.setCiphers(cipherString);

    // Включаем только TLS 1.2 и выше
    config.setProtocol(QSsl::TlsV1_2OrLater);

    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    sslConfig = config;
}

void SslImageClient::fetchImage(const QString &urlString)
{
    QUrl url(urlString);
    if (!url.isValid() || (url.scheme() != "https" && url.scheme() != "http"))
    {
        if (!errorEmitted)
        {
            errorEmitted = true;
            emit errorOccurred("Invalid URL. Use https://host:port/path format");
        }
        return;
    }

    host = url.host();
    port = url.port(443); // По умолчанию порт 443 для HTTPS
    path = url.path();
    if (path.isEmpty())
    {
        path = "/";
    }

    if (socket)
    {
        socket->deleteLater();
    }

    socket = new QSslSocket(this);
    socket->setSslConfiguration(sslConfig);

    responseBuffer.clear();
    headersReceived = false;
    contentLength = 0;
    imageData.clear();
    errorEmitted = false; // Сбрасываем флаг при новом запросе

    connect(socket, &QSslSocket::connected, this, &SslImageClient::onConnected);
    connect(socket, &QSslSocket::readyRead, this, &SslImageClient::onReadyRead);
    connect(socket, &QSslSocket::errorOccurred, this, &SslImageClient::onError);
    connect(socket, &QSslSocket::disconnected, this, &SslImageClient::onDisconnected);

    socket->connectToHostEncrypted(host, port);
}

void SslImageClient::onConnected()
{
    // SSL handshake уже выполнен в connectToHostEncrypted
    // Формируем HTTP GET запрос
    QByteArray request = "GET " + path.toUtf8() + " HTTP/1.1\r\n";
    request += "Host: " + host.toUtf8() + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";

    socket->write(request);
}

void SslImageClient::onReadyRead()
{
    responseBuffer.append(socket->readAll());
    parseHttpResponse();
}

void SslImageClient::parseHttpResponse()
{
    if (!headersReceived)
    {
        // Ищем конец заголовков (\r\n\r\n)
        int headerEnd = responseBuffer.indexOf("\r\n\r\n");
        if (headerEnd == -1)
        {
            return; // Заголовки еще не полностью получены
        }

        // Парсим заголовки
        QByteArray headers = responseBuffer.left(headerEnd);
        QStringList headerLines = QString::fromUtf8(headers).split("\r\n");

        // Парсим первую строку статуса
        if (headerLines.isEmpty())
        {
            socket->disconnectFromHost();
            return;
        }

        QString statusLine = headerLines[0];
        int statusCode = getStatusCode(statusLine);
        if (statusCode != 200)
        {
            QString errorMsg;
            switch (statusCode)
            {
            case 404:
                errorMsg = "Image not found";
                break;
            case 403:
                errorMsg = "Access forbidden";
                break;
            case 500:
                errorMsg = "Internal server error";
                break;
            default:
                errorMsg = QString("Server error: %1 (%2)").arg(statusLine).arg(statusCode);
                break;
            }
            if (!errorEmitted)
            {
                errorEmitted = true;
                emit errorOccurred(errorMsg);
            }
            socket->disconnectFromHost();
            return;
        }

        // Ищем Content-Length
        QString lengthStr = getHeaderValue(headerLines, "Content-Length");
        if (!lengthStr.isEmpty())
        {
            contentLength = lengthStr.toInt();
        }

        // Ищем Content-Type
        QString contentType = getHeaderValue(headerLines, "Content-Type");
        bool isImage = contentType.startsWith("image/jpeg", Qt::CaseInsensitive);

        if (!isImage && contentLength > 0)
        {
            if (!errorEmitted)
            {
                errorEmitted = true;
                emit errorOccurred("Response is not an image");
            }
            socket->disconnectFromHost();
            return;
        }

        responseBuffer.remove(0, headerEnd + 4);
        headersReceived = true;
    }

    imageData.append(responseBuffer);
    responseBuffer.clear();

    if (contentLength > 0 && imageData.size() >= contentLength)
    {
        imageData = imageData.left(contentLength);

        QImage image;
        if (image.loadFromData(imageData, "JPEG"))
        {
            emit imageReceived(image);
        }
        else
        {
            if (!errorEmitted)
            {
                errorEmitted = true;
                emit errorOccurred("Failed to decode JPEG image");
            }
        }

        socket->disconnectFromHost();
    }
}

void SslImageClient::onError(QAbstractSocket::SocketError error)
{
    if (errorEmitted)
    {
        return;
    }

    QString errorMsg;
    switch (error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg = "Connection refused";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorMsg = "Host not found";
        break;
    case QAbstractSocket::NetworkError:
        errorMsg = "Network error";
        break;
    case QAbstractSocket::SslInternalError:
        errorMsg = "SSL internal error";
        break;
    case QAbstractSocket::SslInvalidUserDataError:
        errorMsg = "SSL invalid user data error";
        break;
    default:
        errorMsg = "Socket error: " + QString::number(error);
        break;
    }
    if (socket)
    {
        QString socketError = socket->errorString();
        if (!socketError.isEmpty())
        {
            errorMsg += " - " + socketError;
        }
    }
    errorEmitted = true;
    emit errorOccurred(errorMsg);
}

void SslImageClient::onDisconnected()
{
    if (headersReceived && !imageData.isEmpty())
    {
        // Если Content-Length не был указан, пытаемся загрузить изображение
        if (contentLength == 0)
        {
            QImage image;
            if (image.loadFromData(imageData, "JPEG"))
            {
                emit imageReceived(image);
                return;
            }
        }
    }
}
