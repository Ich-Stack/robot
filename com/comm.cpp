#include "comm.h"

COMM::COMM(QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort();
    m_baudRate = 9600;
    m_serialNumber = "";
}

//这个函数用来设置波特率,打开串口的时候用到
static QSerialPort::BaudRate getBaudRate(int baudRate)
{
    switch (baudRate)
    {
    case 1200:
        return QSerialPort::Baud1200;
    case 2400:
        return QSerialPort::Baud2400;
    case 4800:
        return QSerialPort::Baud4800;
    case 9600:
        return QSerialPort::Baud9600;
    case 19200:
        return QSerialPort::Baud19200;
    case 38400:
        return QSerialPort::Baud38400;
    case 57600:
        return QSerialPort::Baud57600;
    case 115200:
        return QSerialPort::Baud115200;
    default:
        return QSerialPort::UnknownBaud;
    }
}

//这个函数用来设置串口
void COMM::_setPort(const QSerialPortInfo &infobuf)
{
    m_info = infobuf;
}

//用来获取串口的名字
//QString COMM::serialNumber() const
//{
//    return m_serialNumber;
//}

//设置波特率 9600  19200  38400
void COMM::setBaudRate(int baudRate)
{
    m_baudRate = baudRate;
}

//用来打开串口，调用前，先设置串口名字和波特率
bool COMM::open()
{
    if (m_serialPort->isOpen())
    {
        return true;
    }
    m_serialPort->setPort(m_info);
    m_serialPort->setBaudRate(getBaudRate(m_baudRate));
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort->setReadBufferSize(1024);
    return m_serialPort->open(QSerialPort::ReadWrite);
}

//用来关闭串口
void COMM::close()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->close();
    }
}

//重启串口,清楚数据
bool COMM::clear()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->clear();
    }
    return false;
}

//用来接收串口发来的数据
int COMM::readData(char *buffer, int count, int timeout)
{
    int len = 0;
    forever
    {
        int n = m_serialPort->read(&buffer[len], count - len);
        if (n == -1)
        {
            return -1;
        }
        else if (n == 0 && !m_serialPort->waitForReadyRead(timeout))
        {
            return -2;
        }
        else
        {
            len += n;
            if (count == len)
                break;
        }
    }
    return len;
}

//发送数据到串口  比如发送协议
int COMM::writeData(char *data, int size)
{
    int len = 0;
    forever
    {
        int n = m_serialPort->write(&data[len], size - len);
        if (n == -1)
        {
            return -1;
        }
        else
        {
            len += n;
            if (size == len)
                break;
        }
    }
    return len;
}
