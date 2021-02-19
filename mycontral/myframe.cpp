#include "myframe.h"
#include <QPainter>

MYFRAME::MYFRAME(QWidget *parent) : QFrame(parent)
{

}

void MYFRAME::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QColor color(0, 0, 0, 0);
    for(int i=0;i<=10; i++)
    {
        color.setAlpha(i*2);
        painter.setPen(color);
        painter.setBrush(Qt::transparent);
        painter.drawRoundedRect(i,i,this->width()-i*2, this->height()-i*2,15,15);
    }
}
