#ifndef INIT_H
#define INIT_H

#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include "mainwidget.h"

namespace Ui {
class INIT;
}

class INIT : public QWidget
{
    Q_OBJECT

public:
    explicit INIT(QWidget *parent = nullptr);
    ~INIT();

private slots:
    void on_btn_close_clicked();
    void on_btn_start_clicked();
    void on_btn_min_clicked();

signals:
    void btn_start_click();

private:
    Ui::INIT *ui;

private:
    virtual void paintEvent(QPaintEvent *e) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // INIT_H
