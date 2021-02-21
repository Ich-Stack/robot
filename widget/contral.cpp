#include "contral.h"
#include "ui_contral.h"
#include <QDebug>

CONTRAL::CONTRAL(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CONTRAL)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowModality(Qt::ApplicationModal);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("QPushButton{border:0px}");

    ui->btn_cent->setNorAndPre(":/picture/jia.png", ":/picture/fang.png");
    ui->btn_up->setNorAndPre(":/picture/_up.png", ":/picture/up.png");
    ui->btn_down->setNorAndPre(":/picture/_down.png", ":/picture/down.png");
    ui->btn_left->setNorAndPre(":/picture/_left.png", ":/picture/left.png");
    ui->btn_right->setNorAndPre(":/picture/_right.png", ":/picture/right.png");
    ui->btn_esc->setNorAndPre(":/picture/esc.png");
}

CONTRAL::~CONTRAL()
{
    delete ui;
}

void CONTRAL::on_btn_esc_clicked()
{
    this->close();
}

void CONTRAL::keyPressEvent(QKeyEvent *e)
{
    if (e->key() ==  Qt::Key_Up && !e->isAutoRepeat())
    {
        ui->btn_up->keyPress();
        emit up_press();
        return;
    }
    else if (e->key() == Qt::Key_Down && !e->isAutoRepeat())
    {
        ui->btn_down->keyPress();
        emit down_press();
        return;
    }
    else if (e->key() == Qt::Key_Left && !e->isAutoRepeat())
    {
        ui->btn_left->keyPress();
        emit left_press();
        return;
    }
    else if (e->key() == Qt::Key_Right && !e->isAutoRepeat())
    {
        ui->btn_right->keyPress();
        emit right_press();
        return;
    }
    else if (e->key() == Qt::Key_Escape && !e->isAutoRepeat())
    {
        this->close();
    }
    else if (e->key() == Qt::Key_Space && !e->isAutoRepeat())
    {
        if(catFlags)                                            //按一次按键更换图标
        {
            ui->btn_cent->keyPress();
            catFlags = false;
        }
        else
        {
            ui->btn_cent->keyRelease();
            catFlags = true;
        }
        emit cent_press();
        return;
    }
    else
    {
        return;
    }
}

void CONTRAL::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Up && !e->isAutoRepeat())
    {
        ui->btn_up->keyRelease();
        emit stop();
        return;
    }
    else if(e->key() == Qt::Key_Down && !e->isAutoRepeat())
    {
        ui->btn_down->keyRelease();
        emit stop();
        return;
    }
    else if(e->key() == Qt::Key_Left && !e->isAutoRepeat())
    {
        ui->btn_left->keyRelease();
        emit stop();
        return;
    }
    else if(e->key() == Qt::Key_Right && !e->isAutoRepeat())
    {
        ui->btn_right->keyRelease();
        emit stop();
        return;
    }
    else
    {
        return;
    }
}

void CONTRAL::paintEvent(QPaintEvent *)
{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    painter.setBrush(QBrush(QColor(192, 192, 192)));
//    painter.setPen(Qt::transparent);
//    QRect rect = this->rect();
//    rect.setWidth(rect.width() - 1);
//    rect.setHeight(rect.height() - 1);
//    painter.drawRoundedRect(rect, 15, 15);
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

bool CONTRAL::getCatFlags()
{
    return catFlags;
}
