# Wrapper for the Arduino PubSubClient library

## Requirements
* Arduino IDE
* PubSubClient (tested with version 2.6)

## Installation
Simply copy mqtt_handler.h and mqtt_handler.cpp into your project and put `#include "mqtt_handler.h"` in your sketch

## Example
```cpp
#include "mqtt_handler.h"
#include <IpAddress.h> // This is part of Arduino

void setup() {
    // Setup WiFi, etc...

    /*
    // Do something like this to wait for WiFi connection:
    while(!WiFi.isConnected()) {
        delay(100);
    }
    */

    const String my_mqtt_host = "mqtt.domain.tld";
    IpAddress mqtt_ip;
    // Try to resolve DNS. If the host can't be resolved, assume it's an IP address.
    if(WiFi.hostByName(my_mqtt_host, &ip)!=1) {
        mqtt_ip.fromString(my_mqtt_host);
    }
    MQTT.begin(ip, "my-device-id", "username", "password");
    MQTT.handle_topic("topic1", [](char* topic, unsigned char* data, unsigned int len) {
        const String payload_str((char*)data, len);
        Serial.print("Payload: ");
        Serial.println(payload_str);
    });
    MQTT.handle_topic("topic2", [](char* topic, unsigned char* data, unsigned int len) {
        // Dp something with the received data
    });
    MQTT.publish("state", "It's alive!");
}

void loop() {
    // Do whatever your loop needs to do ...

    MQTT.update(); // Call the MQTT update function to handle things.
}

```
