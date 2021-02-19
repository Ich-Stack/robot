#include "dialog.h"
#include "ui_dialog.h"
#include<QIntValidator>
#include <QPaintEvent>
#include <QPainter>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->lineEdit_long->setValidator(new QIntValidator(1, 999, this) );          //基站的输入窗口，输入限制
    ui->lineEdit_wide->setValidator(new QIntValidator(1, 999, this) );
    ui->btn_ok->setNorAndPre(":/yes.png");
    ui->btn_cancel->setNorAndPre(":/no.png");
    this->setWindowFlags(Qt::FramelessWindowHint);
}

Dialog::~Dialog()
{
    delete ui;
}
void Dialog::closeEvent(QCloseEvent *)                                  //关闭窗口事件
{
    ui->lineEdit_long->setFocus();
    isclick = true;
}

void Dialog::on_btn_ok_clicked()
{
    if(ui->lineEdit_long->text().isEmpty() || ui->lineEdit_wide->text().isEmpty())
    {
        length = 1;
        width = 1;
    }
    else
    {
        length = ui->lineEdit_long->text().toDouble();                              //获取用户输入的数值
        width = ui->lineEdit_wide->text().toDouble();
    }
    isclick = true;
    this->close();
}

void Dialog::on_btn_cancel_clicked()
{
    length = 1;
    width = 1;
    isclick = true;
    this->close();
}

double Dialog::getLength()                                                  //接口
{
    return length*1000;
}

double Dialog::getWidth()
{
    return width*1000;
}

void Dialog::setIsClick(bool isclose)
{
    isclick = isclose;
}
bool Dialog::getIsClick()
{
    return isclick;
}

int Dialog::getRatio() const
{
    return length / width;
}
