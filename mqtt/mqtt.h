#ifndef MQTT_H
#define MQTT_H

#include <QObject>
#include <QtMqtt/qmqttclient.h>

class MQTT : public QMqttClient
{
    Q_OBJECT
public:
    explicit MQTT(QMqttClient *parent = nullptr);
    ~MQTT();

    void sub(const QString &topic);
signals:

private:

};

#endif // MQTT_H
