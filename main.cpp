#include <QtWidgets>
#include <QApplication>
#include <Widget.h>


int main(int argc, char** argv) {
    QApplication app(argc, argv);
    Widget* w = new Widget();

    w->show();
    return app.exec();
}