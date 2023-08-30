#include "Mqtt\MeasurementsMqttMessagePublisher.h"

MeasurementsMqttMessagePublisher::MeasurementsMqttMessagePublisher(activatedPidsKeyValuePair *activePids) {
    _activePids = activePids;
}

void MeasurementsMqttMessagePublisher::publishMessage(String topic, int value)
{
    String publishTopic = PubTopic + topic;
    Serial.println("publishing to topic:");
    Serial.println(publishTopic.c_str());
    char sensorValue[12];
    snprintf(sensorValue, sizeof(sensorValue), "%d", value);
    uint16_t packetIdPub1 = mqttClient.publish(publishTopic.c_str(), 0, true, sensorValue);
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
} 