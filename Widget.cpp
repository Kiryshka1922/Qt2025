#include <Widget.h>


Widget::Widget(QWidget* parent) : QWidget(parent) {
    resize(1000, 1000);
    menuMain = new QMenuBar(this);
    menuFile = new QMenu(tr("&File"), this);
    menuMain->addMenu(menuFile);

    actOpen = new QAction(tr("&Open"), this);
    actOpen->setShortcut(QString("Ctrl+0"));
    connect(actOpen, &QAction::triggered, this, &Widget::open);

    menuFile->addAction(actOpen);
    menuFile->addSeparator();
    menuFile->addAction(tr("E&xit"), qApp, &QApplication::quit);
}

void Widget::open() {
    QMessageBox::information(this, tr("Открывашка"), tr("<b>Открывашка</b> делать <i>открывание</i> файла"));
}