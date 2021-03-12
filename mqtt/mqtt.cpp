#include "mqtt.h"
#include <QTime>
#include <QMessageBox>

MQTT::MQTT(QMqttClient *parent) : QMqttClient(parent)
{
    this->setClientId("robot");
    this->setHostname("112.74.84.128");
    this->setPort(1883);
    this->connectToHost();
}

MQTT::~MQTT()
{

}

void MQTT::sub(const QString &top)
{
    QMqttTopicFilter topic(top);
    this->subscribe(topic);
}
