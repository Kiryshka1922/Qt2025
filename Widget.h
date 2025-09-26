#pragma once

#include <QWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QApplication>
#include <QAction>

// class QMenu;
// class QMenuBar;
// class QAction;

class Widget : public QWidget {
    Q_OBJECT

    QMenu    *menuFile;
    QMenuBar *menuMain;
    QAction  *actOpen;
public:
    Widget(QWidget* parent = nullptr);
public slots:
    void open();
};