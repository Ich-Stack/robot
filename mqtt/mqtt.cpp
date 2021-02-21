#include "mqtt.h"
#include <QTime>
#include <QMessageBox>

MQTT::MQTT(QObject *parent) : QObject(parent)
{
    m_client = new QMqttClient(this);
    m_client->setClientId("robot");
    m_client->setHostname("112.74.84.128");
    m_client->setPort(1883);
    m_client->connectToHost();

    connect(m_client, &QMqttClient::messageReceived, this, &MQTT::receiveData);
}

MQTT::~MQTT()
{
    delete m_client;
}

void MQTT::sub(const QString &top)
{
    QMqttTopicFilter topic(top);
    m_client->subscribe(topic);
}

void MQTT::receiveData(const QByteArray &message, const QMqttTopicName &topic)
{
    const QString content = QDateTime::currentDateTime().toString()
                + QLatin1String(" Received Topic: ")
                + topic.name()
                + QLatin1String(" Message: ")
                + message
                + QLatin1Char('\n');
    qDebug() << content;
}
