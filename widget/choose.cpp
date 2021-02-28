#include "choose.h"
#include "ui_choose.h"

CHOOSE::CHOOSE(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CHOOSE)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("QPushButton{border:0px}");

    ui->widget->installEventFilter(this);
    btn_auto = new MYBTN(ui->frame);
    btn_set = new MYBTN(ui->frame);
    btn_auto->move(10, 50);
    btn_auto->resize(70, 45);
    btn_set->move(100, 50);
    btn_set->resize(70, 45);
    btn_set->setNorAndPre(":/picture/btn_set.png");
    btn_auto->setNorAndPre(":/picture/btn_auto.png");
    ui->btn_close->setNorAndPre(":/picture/btn_close.png");
}

CHOOSE::~CHOOSE()
{
    delete ui;
}

bool CHOOSE::eventFilter(QObject *obj, QEvent *evt)
{
    QMouseEvent *mouse =  dynamic_cast<QMouseEvent *>(evt);
    if(obj == ui->widget&&mouse)                 //判断拖动
    {
        if(this->isMaximized())
        {
            return true;
        }
        static bool dragFlag = false;
        static QPoint dragPoint(0,0);
         if(mouse->button()==Qt::LeftButton && mouse->type() ==QEvent::MouseButtonPress)    //按下
        {
            dragFlag =true;
            dragPoint = mouse->pos();                                  //记录鼠标所在的界面位置
            return true;
        }
        else if(dragFlag &&  mouse->type() ==QEvent::MouseMove)     //拖动
        {
            this->move(mouse->globalPos() - dragPoint);
            return true;
        }
        else if(mouse->type() ==QEvent::MouseButtonRelease)
        {
            dragFlag = false;
            return true;
        }
    }
    return QWidget::eventFilter(obj,evt);
}

void CHOOSE::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QColor color(0, 0, 0, 0);
    for(int i=0;i<=10; i++)
    {
        color.setAlpha(i*4);
        painter.setPen(color);
        painter.setBrush(Qt::transparent);
        painter.drawRoundedRect(i,i,this->width()-i*2, this->height()-i*2,15,15);
    }
}

void CHOOSE::on_btn_close_clicked()
{
    this->close();
}
