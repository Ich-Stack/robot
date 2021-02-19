#ifndef COMM_H
#define COMM_H

#include <QObject>
#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class COMM : public QObject
{
    Q_OBJECT
public:
    explicit COMM(QObject *parent = nullptr);
    //串口部分
    bool isOpen = false;
    int write(char ch);
    int baudRate() const;
    int writeData(char *data, int size);
    int readData(char *buffer, int count, int timeout = 1000);
    virtual void close();
    virtual bool clear();
    virtual bool open();
    void setBaudRate(int baudRate);
    void _setPort(const QSerialPortInfo &);
    //void setPort
    //QString serialNumber() const;
    QSerialPort *m_serialPort;

protected:
    int m_baudRate;
    QString m_serialNumber;
    QSerialPortInfo m_info;

signals:
};

#endif // COMM_H
