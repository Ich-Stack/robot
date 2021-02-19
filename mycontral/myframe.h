#ifndef MYFRAME_H
#define MYFRAME_H

#include <QWidget>
#include <QFrame>

class MYFRAME : public QFrame
{
    Q_OBJECT
public:
    explicit MYFRAME(QWidget *parent = nullptr);

signals:

private:
    virtual void paintEvent(QPaintEvent *) override;
};

#endif // MYFRAME_H
