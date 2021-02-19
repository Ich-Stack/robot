#include "init.h"
#include "ui_init.h"
#include <QTimer>

INIT::INIT(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::INIT)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    this->installEventFilter(this);
    this->setStyleSheet("QWidget{border:0px}");
    this->setStyleSheet("QPushButton{border:0px}");
    ui->btn_start->setNorAndPre(":/btn_start.png");
    ui->btn_min->setNorAndPre(":/btn_min.png");
    ui->btn_close->setNorAndPre(":/btn_close.png");
}

INIT::~INIT()
{
    delete ui;
}

void INIT::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.drawPixmap(rect(),QPixmap(":/init.png"),QRect());
}

bool INIT::eventFilter(QObject *obj, QEvent *evt)
{
    QMouseEvent *mouse =  dynamic_cast<QMouseEvent *>(evt);
    if(obj == this&&mouse)                 //判断拖动
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

void INIT::on_btn_close_clicked()
{
    this->close();
}

void INIT::on_btn_start_clicked()
{
    QTimer::singleShot(200, this, [=](){
        emit btn_start_click();
    });
}

void INIT::on_btn_min_clicked()
{
    this->showMinimized();
}
