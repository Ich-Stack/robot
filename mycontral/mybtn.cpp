#include "mybtn.h"
#include <QRect>
#include <QTimer>
#include <QPropertyAnimation>

MYBTN::MYBTN(QWidget *parent) : QPushButton(parent)
{
    this->setFocusPolicy(Qt::NoFocus);
    connect(this, &QPushButton::clicked, this, &MYBTN::click_zoom);
    //connect(this, &MYBTN::clicked, this, &MYBTN::slot_delay);
}

void MYBTN::display(QPixmap &pix)
{
    pix.scaled(this->size(), Qt::KeepAspectRatio);
    this->setIconSize(QSize(this->width(), this->height()));
    this->setIcon(pix);
}

void MYBTN::setNorAndPre(QString _nor, QString _pre)
{
    nor.load(_nor);
    pre.load(_pre);
    display(nor);
}

void MYBTN::zoom_rise()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);                             //时间
    animation->setStartValue(QRect(this->x(), this->y()+3, this->width(), this->height()));
    animation->setEndValue(QRect(this->x(), this->y(), this->width(), this->height()));  //位置
    animation->setEasingCurve(QEasingCurve::OutBounce);      //弹跳曲线
    animation->start();
}

void MYBTN::zoom_down()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);                             //时间
    animation->setStartValue(QRect(this->x(), this->y(), this->width(), this->height()));
    animation->setEndValue(QRect(this->x(), this->y()+3, this->width(), this->height()));  //位置
    animation->setEasingCurve(QEasingCurve::OutBounce);      //弹跳曲线
    animation->start();
}

void MYBTN::click_zoom()
{
    zoom_down();
    zoom_rise();
}

//void MYBTN::slot_delay()
//{
//    QTimer::singleShot(200, [=](){
//        emit this->released();
//    });
//}

void MYBTN::keyPress()
{
    display(pre);
}

void MYBTN::keyRelease()
{
    display(nor);
}
