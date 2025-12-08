#include <QApplication>
#include "../../ui/client/ssl_client_mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SslClientMainWindow w;
    w.setWindowTitle("Lab22 - SSL JPEG Image Client (GOST)");
    w.show();
    return a.exec();
}

