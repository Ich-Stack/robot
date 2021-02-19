#ifndef CONTRAL_H
#define CONTRAL_H

#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QKeyEvent>
#include <QPainter>

namespace Ui {
class CONTRAL;
}

class CONTRAL : public QWidget
{
    Q_OBJECT

public:
    ~CONTRAL();
    explicit CONTRAL(QWidget *parent = nullptr);
    bool getCatFlags();

signals:
    void up_press();
    void down_press();
    void left_press();
    void right_press();
    void cent_press();
    void stop();

private slots:
    void on_btn_esc_clicked();

private:
    Ui::CONTRAL *ui;

    bool catFlags = true;
private:
    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void keyReleaseEvent(QKeyEvent *) override;
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // CONTRAL_H
