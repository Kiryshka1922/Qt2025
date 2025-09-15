#include <QWidget

#include "IMage.h"

class Widget: public QWidget
{
  Q_OBJECT
  IMageWidget & image;
  Widget(QWidget *parent):QWidget (parent)
  
  {
    image = new ImageWidget();
    button = new QPushButton(tr("Open"));
    connect (&button, &QPushButton::clicked,this, &Widget::open);
    QVBoxLayout *l = new QWBxLayout(this);
    l -> addWidget(button);
    l -> addWidget(image);
  }
  public slots:

  void open(){
    QString filename = QFileDialog::getOpenFileName(this, tr('Open'), ".", "JPEG Files(*.jpg; *jpeg)");
  }
}
