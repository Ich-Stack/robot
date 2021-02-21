#ifndef MQTT_H
#define MQTT_H

#include <QObject>
#include <QtMqtt/qmqttclient.h>

class MQTT : public QObject
{
    Q_OBJECT
public:
    explicit MQTT(QObject *parent = nullptr);
    ~MQTT();

    void sub(const QString &topic);
    void receiveData(const QByteArray &message, const QMqttTopicName &topic);
signals:

private:
    QMqttClient *m_client;

};

#endif // MQTT_H
