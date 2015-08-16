#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "secrets.h"
const int feedButtonPin = 0;
const int publishTimeout = 2000;
const int feedButtonHoldTime = 50;
const String payload = "penni";
char* topic = "control/button";
String clientName;

WiFiClient wifiClient;
PubSubClient client(server, port, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void setup() {
  pinMode(feedButtonPin, INPUT_PULLUP);
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Generate client name based on MAC address and last 8 bits of microsecond counter
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  
  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);
    
    if (client.publish(topic, "control/button \"penni\" online")) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }
  } else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
}

void loop() {
  checkButton();
}

void checkButton() {
  if (digitalRead(feedButtonPin) == LOW) {
    long feedButtonPressedTime = millis();
    while (digitalRead(feedButtonPin) == LOW) {
      if (feedButtonPressedTime + feedButtonHoldTime < millis()) {
        if (client.connected()){
         sendMQTTMessage();
        } else {
          Serial.println("Disconnected... reconnecting");
          if (client.connect((char*) clientName.c_str())) {
            Serial.println("Connected to MQTT broker");
            sendMQTTMessage();
          };
        };
        while (digitalRead(feedButtonPin) == LOW) {
          delay(100);
        }; //Prevents multiple publishings
      }
    }
  }
}

void sendMQTTMessage() {
  Serial.print("Sending payload: ");
  Serial.println(payload);
  if (client.publish(topic, (char*) payload.c_str())) {
    Serial.println("Publish ok");
  } else {
    Serial.println("Publish failed");
  }
}
