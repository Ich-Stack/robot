#ifndef CHOOSE_H
#define CHOOSE_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include "mycontral/mybtn.h"

namespace Ui {
class CHOOSE;
}

class CHOOSE : public QWidget
{
    Q_OBJECT

public:
    explicit CHOOSE(QWidget *parent = nullptr);
    ~CHOOSE();

    MYBTN *btn_auto;
    MYBTN *btn_set;

private slots:
    void on_btn_close_clicked();

private:
    Ui::CHOOSE *ui;

    virtual void paintEvent(QPaintEvent *e) override;
    virtual bool eventFilter(QObject *obj, QEvent *evt) override;
};

#endif // CHOOSE_H
