#ifndef CHOOSE_H
#define CHOOSE_H

#include <QWidget>

namespace Ui {
class CHOOSE;
}

class CHOOSE : public QWidget
{
    Q_OBJECT

public:
    explicit CHOOSE(QWidget *parent = nullptr);
    ~CHOOSE();

private:
    Ui::CHOOSE *ui;
};

#endif // CHOOSE_H
