#include "choose.h"
#include "ui_choose.h"

CHOOSE::CHOOSE(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CHOOSE)
{
    ui->setupUi(this);
}

CHOOSE::~CHOOSE()
{
    delete ui;
}
