#include "state.h"
#include "ui_state.h"

STATE::STATE(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::STATE)
{
    ui->setupUi(this);
}

STATE::~STATE()
{
    delete ui;
}
