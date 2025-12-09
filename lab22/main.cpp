#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Lab22 - Qt + client_server_ssl");
    w.show();
    return a.exec();
}
