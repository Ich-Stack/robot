#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void closeEvent(QCloseEvent *);
    double getLength();
    double getWidth();
    void setIsClick(bool isclose);                             //关闭串口设置isclick为false
    bool getIsClick();
    double getRatio() const;
    uint getArriveDis();
    uint getLandingDis();

private slots:
    void on_btn_ok_clicked();
    void on_btn_cancel_clicked();

private:
    Ui::Dialog *ui;

    bool isclick = false;
    double length = 1;
    double width = 1;

private:
};

#endif // DIALOG_H
