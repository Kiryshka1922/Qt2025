#include <Widget.h>
#include <QWidget>


Widget::Widget(QWidget* parent){
  
}
void Widget:paintEvent(QPaintEvent*){
  QPainter painter(this);
  painter.drawPixmap(pixmap);
}