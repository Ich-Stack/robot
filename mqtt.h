#ifndef MQTT_H
#define MQTT_H

#include <QObject>

class MQTT : public QObject
{
    Q_OBJECT
public:
    explicit MQTT(QObject *parent = nullptr);

signals:

};

#endif // MQTT_H
