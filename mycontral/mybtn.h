#ifndef MYBTN_H
#define MYBTN_H

#include <QWidget>
#include <QPushButton>
#include <QKeyEvent>

class MYBTN : public QPushButton
{
    Q_OBJECT
public:
    explicit MYBTN(QWidget *parent = nullptr);

    void zoom_down();
    void zoom_rise();
    void keyPress();
    void keyRelease();
    void click_zoom();
    void setNorAndPre(QString _nor, QString _pre = "");

signals:

private:
    QPixmap nor;
    QPixmap pre;

private:
    void display(QPixmap &pix);
};

#endif // MYBTN_H
