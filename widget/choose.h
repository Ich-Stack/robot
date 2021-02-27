#ifndef CHOOSE_H
#define CHOOSE_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>

namespace Ui {
class CHOOSE;
}

class CHOOSE : public QWidget
{
    Q_OBJECT

public:
    explicit CHOOSE(QWidget *parent = nullptr);
    ~CHOOSE();

    QPushButton *btn_auto;
    QPushButton *btn_set;

private:
    Ui::CHOOSE *ui;

    virtual void paintEvent(QPaintEvent *e) override;
    virtual bool eventFilter(QObject *obj, QEvent *evt) override;
};

#endif // CHOOSE_H
