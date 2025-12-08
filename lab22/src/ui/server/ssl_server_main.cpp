#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QDir>
#include "../../network/server/ssl_image_server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    QWidget window;
    window.setWindowTitle("Lab22 - SSL JPEG Image Server (GOST)");
    window.resize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&window);
    
    QHBoxLayout *portLayout = new QHBoxLayout;
    portLayout->addWidget(new QLabel("Port:"));
    QSpinBox *portSpin = new QSpinBox;
    portSpin->setRange(1024, 65535);
    portSpin->setValue(8443);
    portLayout->addWidget(portSpin);
    portLayout->addStretch();
    layout->addLayout(portLayout);
    
    QHBoxLayout *dirLayout = new QHBoxLayout;
    dirLayout->addWidget(new QLabel("Image Directory:"));
    QLineEdit *dirEdit = new QLineEdit;
    dirEdit->setText(QDir::currentPath());
    dirLayout->addWidget(dirEdit);
    layout->addLayout(dirLayout);
    
    QPushButton *startButton = new QPushButton("Start SSL Server");
    QPushButton *stopButton = new QPushButton("Stop Server");
    stopButton->setEnabled(false);
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    layout->addLayout(buttonLayout);
    
    QLabel *statusLabel = new QLabel("Status: Stopped");
    layout->addWidget(statusLabel);
    
    QTextEdit *logText = new QTextEdit;
    logText->setReadOnly(true);
    layout->addWidget(logText);
    
    SslImageServer *server = new SslImageServer(&window);
    
    QObject::connect(startButton, &QPushButton::clicked, [&]() {
        quint16 port = static_cast<quint16>(portSpin->value());
        QString dir = dirEdit->text();
        server->setImageDirectory(dir);
        
        if (server->startServer(port)) {
            statusLabel->setText(QString("Status: Running on port %1 (SSL/TLS with GOST)").arg(port));
            logText->append(QString("SSL Server started on port %1, serving from: %2")
                           .arg(port).arg(dir));
            logText->append("SSL/TLS encryption enabled with GOST cipher support");
            logText->append("Note: Full GOST support requires GOST-enabled OpenSSL or CryptoPro CSP");
            startButton->setEnabled(false);
            stopButton->setEnabled(true);
            portSpin->setEnabled(false);
            dirEdit->setEnabled(false);
        } else {
            QMessageBox::warning(&window, "Error", "Failed to start SSL server");
        }
    });
    
    QObject::connect(stopButton, &QPushButton::clicked, [&]() {
        server->close();
        statusLabel->setText("Status: Stopped");
        logText->append("SSL Server stopped");
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
        portSpin->setEnabled(true);
        dirEdit->setEnabled(true);
    });
    
    window.show();
    return a.exec();
}

