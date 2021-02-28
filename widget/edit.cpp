#include "edit.h"
#include "ui_edit.h"
#include <QPainter>

EDIT::EDIT(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EDIT)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet("QPushButton{border:0px}");

    ui->widget->installEventFilter(this);
    ui->btn_close->setNorAndPre(":/picture/btn_close.png");
    ui->btn_addcoor->setNorAndPre(":/picture/btn_addcoor.png");
    ui->btn_addtask->setNorAndPre(":/picture/btn_addtask.png");

    btn_load = new MYBTN(ui->frame);
    btn_save = new MYBTN(ui->frame);
    btn_load->setNorAndPre(":/picture/btn_load.png");
    btn_save->setNorAndPre(":/picture/btn_save.png");
    btn_load->move(290, 100);
    btn_load->resize(90, 25);
    btn_save->move(290, 150);
    btn_save->resize(90, 25);

    QRegularExpressionValidator *regExpress = new QRegularExpressionValidator(QRegularExpression("^\\d{0,3}$"));
    ui->lineEdit_taskcode->setValidator(regExpress);

    connect(btn_save, &QPushButton::clicked, this, [=](){this->close();});
    connect(btn_load, &QPushButton::clicked, this, [=](){this->close();});
}

EDIT::~EDIT()
{
    delete ui;
}

bool EDIT::eventFilter(QObject *obj, QEvent *evt)
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

void EDIT::paintEvent(QPaintEvent *)
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
void EDIT::lineEdit_taskcode_clear()
{
    ui->lineEdit_taskcode->clear();
}
void EDIT::lineEdit_taskname_clear()
{
    ui->lineEdit_taskname->clear();
}
QString EDIT::LineEdit_taskname_text()
{
    return ui->lineEdit_taskname->text();
}
QString EDIT::LineEdit_taskcode_text()
{
    return ui->lineEdit_taskcode->text();
}
QString EDIT::cmb_level_text()
{
    return ui->cmb_level->currentText();
}
QString EDIT::cmb_work_text()
{
    return ui->cmb_work->currentText();
}
int EDIT::cmb_level_index()
{
    return ui->cmb_level->currentIndex();
}
int EDIT::cmb_work_index()
{
    return ui->cmb_work->currentIndex();
}

void EDIT::on_btn_close_clicked()
{
    ui->lineEdit_taskname->setFocus();
    this->close();
}

void EDIT::on_btn_addcoor_clicked()
{
    emit btn_addcoor();
}

void EDIT::on_btn_addtask_clicked()
{
    ui->lineEdit_taskname->setFocus();
    emit btn_addtask();
}
