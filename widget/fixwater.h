#ifndef FIXWATER_H
#define FIXWATER_H

#include <QWidget>

namespace Ui {
class FIXWATER;
}

class FIXWATER : public QWidget
{
    Q_OBJECT

public:
    explicit FIXWATER(const QString &id, QWidget *parent = nullptr);
    ~FIXWATER();

    void setTemperatureValue(const QByteArray &val);
    void setTurbidityValue(const QByteArray &val);
    void setpHValue(const QByteArray &val);
private:
    Ui::FIXWATER *ui;
};

#endif // FIXWATER_H
