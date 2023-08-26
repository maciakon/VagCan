#include "Mqtt\MeasurementsMqttMessagePublisher.h"

MeasurementsMqttMessagePublisher::MeasurementsMqttMessagePublisher(activatedPidsKeyValuePair *activePids) {
    _activePids = activePids;
}

void MeasurementsMqttMessagePublisher::publishMessage(String topic, byte value)
{
    String publishTopic = PubTopic + topic;
    Serial.println("pubishing to topic:");
    Serial.println(publishTopic.c_str());
    char sensorValue[4];
    sprintf_P(sensorValue, "%d", value);
    uint16_t packetIdPub1 = mqttClient.publish(publishTopic.c_str(), 0, true, sensorValue);
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
}
