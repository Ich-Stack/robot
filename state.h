#ifndef STATE_H
#define STATE_H

#include <QWidget>

namespace Ui {
class STATE;
}

class STATE : public QWidget
{
    Q_OBJECT

public:
    explicit STATE(QWidget *parent = nullptr);
    ~STATE();

private:
    Ui::STATE *ui;
};

#endif // STATE_H
