#include "get_request_handler.h"
#include "../server/ssl_image_server.h"
#include "../../core/jpeghandler.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>

GetRequestHandler::GetRequestHandler(SslImageServer *sslServer)
    : sslServer(sslServer)
{
}

bool GetRequestHandler::canHandle(const QString &method, const QString &path) {
    Q_UNUSED(path);
    return method == "GET";
}

void GetRequestHandler::handleRequest(QTcpSocket *socket, const QString &method, const QString &path) {
    Q_UNUSED(method);
    
    QString cleanPath = path;
    if (cleanPath.startsWith("/")) {
        cleanPath = cleanPath.mid(1);
    }
    
    if (cleanPath.isEmpty() || cleanPath == "/") {
        sendDirectoryListing(socket);
        return;
    }

    QString imageDir = getImageDirectory();
    QString filePath = QDir(imageDir).absoluteFilePath(cleanPath);
    QFileInfo fileInfo(filePath);
    
    // Проверка безопасности
    if (!filePath.startsWith(QDir(imageDir).absolutePath())) {
        sendErrorResponse(socket, 403, "Forbidden");
        socket->disconnectFromHost();
        return;
    }

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        sendErrorResponse(socket, 404, "Not Found");
        socket->disconnectFromHost();
        return;
    }

    // Проверяем, что это JPEG файл
    JpegHandler handler;
    if (!handler.isValid(filePath)) {
        sendErrorResponse(socket, 400, "Bad Request - Not a valid JPEG file");
        socket->disconnectFromHost();
        return;
    }

    sendImageResponse(socket, filePath);
}

void GetRequestHandler::sendImageResponse(QTcpSocket *socket, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        sendErrorResponse(socket, 500, "Internal Server Error");
        return;
    }

    QByteArray imageData = file.readAll();
    file.close();

    QByteArray response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += QString("Content-Length: %1\r\n").arg(imageData.size()).toUtf8();
    response += "Connection: close\r\n";
    response += "\r\n";
    response += imageData;

    socket->write(response);
    socket->disconnectFromHost();
}

QString GetRequestHandler::getImageDirectory() const {
    if (sslServer) {
        return sslServer->getImageDirectory();
    }
    return QDir::currentPath();
}

void GetRequestHandler::sendDirectoryListing(QTcpSocket *socket) {
    QString imageDir = getImageDirectory();
    QDir dir(imageDir);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg";
    QStringList files = dir.entryList(filters, QDir::Files);
    
    QString html = "<html><body><h1>Available Images</h1><ul>";
    for (const QString &file : files) {
        html += QString("<li><a href=\"/%1\">%1</a></li>").arg(file);
    }
    html += "</ul></body></html>";
    
    QByteArray response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=utf-8\r\n";
    response += QString("Content-Length: %1\r\n").arg(html.toUtf8().size()).toUtf8();
    response += "\r\n";
    response += html.toUtf8();
    
    socket->write(response);
    socket->disconnectFromHost();
}

void GetRequestHandler::sendErrorResponse(QTcpSocket *socket, int code, const QString &message) {
    QString statusText;
    switch (code) {
        case 400: statusText = "Bad Request"; break;
        case 403: statusText = "Forbidden"; break;
        case 404: statusText = "Not Found"; break;
        case 500: statusText = "Internal Server Error"; break;
        default: statusText = "Error"; break;
    }

    QString html = QString("<html><body><h1>%1 %2</h1><p>%3</p></body></html>")
                   .arg(code).arg(statusText).arg(message);
    
    QByteArray response = QString("HTTP/1.1 %1 %2\r\n").arg(code).arg(statusText).toUtf8();
    response += "Content-Type: text/html; charset=utf-8\r\n";
    response += QString("Content-Length: %1\r\n").arg(html.toUtf8().size()).toUtf8();
    response += "Connection: close\r\n";
    response += "\r\n";
    response += html.toUtf8();
    
    socket->write(response);
    socket->disconnectFromHost();
}

