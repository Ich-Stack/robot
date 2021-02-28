#ifndef EDIT_H
#define EDIT_H

#include <QPushButton>
#include <QPaintEvent>
#include <QWidget>
#include "mycontral/mybtn.h"

namespace Ui {
class EDIT;
}

class EDIT : public QWidget
{
    Q_OBJECT

public:
    explicit EDIT(QWidget *parent = nullptr);
    ~EDIT();

    QString LineEdit_taskname_text();
    QString LineEdit_taskcode_text();
    QString cmb_level_text();
    QString cmb_work_text();
    void lineEdit_taskcode_clear();
    void lineEdit_taskname_clear();
    int cmb_level_index();
    int cmb_work_index();
    MYBTN *btn_load;
    MYBTN *btn_save;

signals:
    void btn_addcoor();
    void btn_addtask();

private slots:
    void on_btn_close_clicked();
    void on_btn_addcoor_clicked();
    void on_btn_addtask_clicked();

private:
    Ui::EDIT *ui;

private:
    virtual void paintEvent(QPaintEvent *e) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // EDIT_H
