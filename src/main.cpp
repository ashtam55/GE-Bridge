#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include "kaaro_utils.cpp"

String toSend;
String DEVICE_MAC_ADDRESS;

const int LED = 2;

String META_ROOT = "Kento/present/";
String ROOT_MQ_ROOT = "ge/";
String PRODUCT_MQ_SUB = "gebridge/";
String MESSAGE_MQ_STUB = "message";
String COUNT_MQ_STUB = "count";
String OTA_MQ_SUB = "ota/";

String presenceTopic;
String presenceDemandTopic;

String rootTopic;
String readyTopic;

String otaTopic;

String productMessageTopic;
String productCountTopic;

String messageTopic;
String countTopic;
String msg; //mqtt payload

const char *mqtt_server = "api.akriya.co.in";
void mqttCallback(char *topic, byte *payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqttClient(mqtt_server, 1883, mqttCallback, wifiClient);
WiFiManager wifiManager;

void printToSerial2(String payload){
  if(Serial.available()) {
    if(Serial.print(payload.c_str())){
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000); 
    }
  }
}
void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  char *cleanPayload = (char *)malloc(length + 1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length + 1);
  msg = String(cleanPayload);
  free(cleanPayload);

  String topics = String(topic);
  Serial2.printf("From MQTT = ");
  Serial2.println(msg);

  // String countTopic = ROOT_MQ_ROOT + COUNT_MQ_STUB + DEVICE_MAC_ADDRESS;

  if (topics == productMessageTopic)
  {
      printToSerial2(msg);
  }
  
}

void mqttSetTopicValues() {
  presenceTopic = META_ROOT + DEVICE_MAC_ADDRESS;
  presenceDemandTopic = META_ROOT + "REPORT";
  
  rootTopic = ROOT_MQ_ROOT;
  readyTopic = ROOT_MQ_ROOT + DEVICE_MAC_ADDRESS;

  otaTopic = ROOT_MQ_ROOT + OTA_MQ_SUB + DEVICE_MAC_ADDRESS;

  productMessageTopic = ROOT_MQ_ROOT + PRODUCT_MQ_SUB + MESSAGE_MQ_STUB;
  productCountTopic = ROOT_MQ_ROOT + PRODUCT_MQ_SUB + COUNT_MQ_STUB;

  messageTopic = ROOT_MQ_ROOT + MESSAGE_MQ_STUB + '/' + DEVICE_MAC_ADDRESS;
  countTopic = ROOT_MQ_ROOT + COUNT_MQ_STUB + '/' + DEVICE_MAC_ADDRESS;
}

void reconnect()
{

  if (!mqttClient.connected())
  {
    Serial2.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
      Serial2.println("connected");

      String readyMessage = DEVICE_MAC_ADDRESS + " is Ready.";
      mqttClient.publish(readyTopic.c_str(), "Ready!");
      mqttClient.publish(rootTopic.c_str(), readyMessage.c_str());
      mqttClient.publish(presenceTopic.c_str(), "0");

      mqttClient.subscribe(rootTopic.c_str());
      mqttClient.subscribe(otaTopic.c_str());
      mqttClient.subscribe(presenceDemandTopic.c_str());
      mqttClient.subscribe(productMessageTopic.c_str());
      mqttClient.subscribe(productCountTopic.c_str());

      mqttClient.subscribe(messageTopic.c_str());
      mqttClient.subscribe(countTopic.c_str());
    }

    else
    {
      Serial2.print("failed, rc=");
      Serial2.print(mqttClient.state());
      Serial2.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void WiFiReconnect(){
  // wifi_config_t conf;
  // esp_wifi_get_config(WIFI_IF_STA, &conf);
  // pass =  String(reinterpret_cast<char*>(conf.sta.password));
  // Serial.printf("Pass : %s", pass);
  // WiFi.disconnect();
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid.c_str(),pass.c_str());
if (wifiManager.getWiFiIsSaved()){
      // wifiManager.stopConfigPortal();
    wifiManager.autoConnect("Digital Icon");
}
    if (WiFi.status() == WL_CONNECTED)
  {
    Serial2.println("");
    Serial2.println("WiFi connected");
    Serial2.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial2.println(ip);

  }
}

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  Serial2.begin(9600);
  DEVICE_MAC_ADDRESS = KaaroUtils::getMacAddress();
  mqttSetTopicValues();
  Serial2.println(DEVICE_MAC_ADDRESS);

  Serial2.println("Serial Txd is on pin: "+String(TX));
  Serial2.println("Serial Rxd is on pin: "+String(RX));
  pinMode(LED,OUTPUT);
  digitalWrite(LED, HIGH);   
  delay(1000);                   
  digitalWrite(LED, LOW);    
  delay(1000); 

  Serial2.print("Connecting Wifi: ");
  wifiManager.setConnectTimeout(5);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setWiFiAutoReconnect(true);
  wifiManager.autoConnect("Digital Icon");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial2.println("");
    Serial2.println("WiFi connected");
    Serial2.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial2.println(ip);
  }
  else{
    WiFiReconnect();
  }

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}
 
void loop() { //Choose Serial1 or Serial2 as required

  wifiManager.process();
 if (WiFi.status() == WL_CONNECTED)
  {

    if (!mqttClient.connected())
    {
      reconnect();
    }
      mqttClient.loop();
  }
  else{
    WiFiReconnect();
  }  
}
