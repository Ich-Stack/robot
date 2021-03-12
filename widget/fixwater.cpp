#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")                       //解决中文乱码问题
#endif

#include "fixwater.h"
#include "ui_fixwater.h"

FIXWATER::FIXWATER(const QString &id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FIXWATER)
{
    ui->setupUi(this);

    this->resize(85, 75);
    ui->ph_key->setAttribute(Qt::WA_TranslucentBackground);
    ui->ph_value->setAttribute(Qt::WA_TranslucentBackground);
    ui->t_key->setAttribute(Qt::WA_TranslucentBackground);
    ui->t_value->setAttribute(Qt::WA_TranslucentBackground);
    ui->z_key->setAttribute(Qt::WA_TranslucentBackground);
    ui->z_value->setAttribute(Qt::WA_TranslucentBackground);
    ui->id_key->setAttribute(Qt::WA_TranslucentBackground);
    ui->id_value->setAttribute(Qt::WA_TranslucentBackground);
    ui->id_value->setText(id);
}

FIXWATER::~FIXWATER()
{
    delete ui;
}
void FIXWATER::setTemperatureValue(const QByteArray &val)
{
    QString text(val + "℃");
    ui->t_value->setText(text);
}
void FIXWATER::setTurbidityValue(const QByteArray &val)
{
    ui->z_value->setText(val + "TU");
}
void FIXWATER::setpHValue(const QByteArray &val)
{
    ui->ph_value->setText(val);
}
