/**
 * HEIG-VD IOT Project
 * Auteurs:
 * Anthony Coke
 * Guilain Mbayo
 * Mehdi Salhi
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include "secret.h"

#define fanON   255
#define fanOFF  0
#define fanPIN  A3
#define moistPin A6

MKRIoTCarrier carrier; 

char ssid[] = SECRET_SSID;       // your network SSID (name)
char pass[] = SECRET_PASS;       // your network password 
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
IPAddress commander(192,168,9,194); //your central server address

// adresse and port of the MQTT broker
const char broker[] = "192.168.9.194";
int        port     = 1883;

String location = "serre_1";
String topicSub  = "commander/devices/"; 
String topicPub  = "arduino";
String confTopic = "";
String deviceUID = "";

// Sensors
#define tempIOTC
#define humiIOTC
#define lighIOTC
#define humiSOIL

int r, g, b, light;

//set initial interval for sending messages (milliseconds)
long interval = 20000;
unsigned long previousMillis = 0;
int count = 0;

DynamicJsonDocument config(1024);

void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (!carrier.begin()) {
    Serial.println("Failed to initialize!");
    while (1);
  }

  delay(1000);
  carrier.display.setTextSize(3);

  carrier.display.setRotation(0);
  delay(1500);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("\nStarting connection to server...");
  byte mac[6];
  WiFi.macAddress(mac);
  for(int i = 0; i < 6; ++i){
    deviceUID += mac[i];
  }
  Serial.println();

  Serial.print("Your device ID is: ");
  Serial.println(deviceUID);
  topicSub += deviceUID + "/#";

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");

  Serial.println();
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  printWifiStatus();

  // device initial configuration
  config["deviceUID"] = deviceUID;
  config["sensors"] = "[\"tempIOTC\", \"humiIOTC\", \"lighIOTC\", \"humiSOIL\", \"movePIR\"]";
  config["measurement interval"] = interval;
  config["deviceLocation"] = "serre_1";
  config["actions"] = "";

  confTopic = "commander/devices/" + deviceUID;
  String conf;
  serializeJson(config, conf);

  // publish the initial configuration
  publishTopics(confTopic, conf);

  // set the message receive callback
  mqttClient.onMessage(listenTopics);

  Serial.print("Subscribing to topic: ");
  Serial.println(topicSub);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topicSub);
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    sendData();
  }
}

// callback function that listen to an MQTT topic
void listenTopics(int messageSize){
  String topic = mqttClient.messageTopic();
  String subTop = "";
  int index_1 = 0;
  int index_2 = 0;
  String content = "";
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic \n'");
  Serial.print(topic);      
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  
  while (mqttClient.available()) {
    content += String((char)mqttClient.read());
  }
  Serial.println(content);
  
  // Decompose the topic
  do{
    if(index_2 == 0){
      index_1 = topic.indexOf('/', 0);
      index_2 = topic.indexOf('/', index_1 + 1);
      subTop = topic.substring(0, index_1);
    } else {
      index_1 = topic.indexOf('/', index_2);
      index_2 = topic.indexOf('/', index_1 + 1);
      subTop = topic.substring(index_1 + 1, index_2);
    }

    if(subTop == String("update")){
      // read the action and the new interval and set it in the configuration
      deserializeJson(config, content);
      if(config["actions"] == "ON"){
        Serial.println("fan ON");        
        fanCtrl(fanON);
      } else if(config["actions"] == "OFF"){
        Serial.println("fan OFF");
        fanCtrl(fanOFF);
      } else {Serial.println("Nothing to see...");} // Add new features here
      interval = config["measurement interval"];      

      // send the updated config
      String conf;
      serializeJson(config, conf);
      publishTopics(confTopic, conf);
    } else if (subTop == "test"){
      Serial.println("This is a test");
    }
  }while(index_1 < index_2);
  
  
  Serial.println();
  Serial.println();
}

// publish a string on an MQTT topic
void publishTopics(String topic, String value){
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(value);
  Serial.println();

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic);
  mqttClient.print(value);
  mqttClient.endMessage();
  delay(500);
}

// print the status of the connected wifi
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


// get data from sensors
// use defines here : #ifdef tempIOTC ...#endif
// so that we get data from sensors according to what was defined at the top

void sendData() {
  // data that will be formatted and sent to the server
  // append sensor data conditionnaly
  
  #ifdef tempIOTC
    String temp = getTemp();
    publishTopics(topicPub, formatLineProtocol("temperature",temp));
  #endif
  
  #ifdef humiIOTC
    String humi = getHumi();
    publishTopics(topicPub, formatLineProtocol("humidity", humi));
  #endif

  #ifdef lighIOTC
    getLight();
    publishTopics(topicPub, formatLineProtocol("light", String(light)));
  #endif

  #ifdef humiSOIL
    String humiSoil = getSoilHum();
    publishTopics(topicPub, formatLineProtocol("soil_humidity", humiSoil));
  #endif

  // should be used to display the return of getData() on the MKR screen
  carrier.display.setCursor(0, 0);
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.print("tempIOTC\n" + String(temp) + "\n" + "humiIOTC\n" + humi + "\n" + "lighIOTC\n" + light + "\n" + "humiSOIL\n" + humiSoil + "\n");
  
}

// get temperature from IOT Carrier
String getTemp() {
  float temp = carrier.Env.readTemperature();
  return  String(temp);
}

// get humidity from IOT Carrier
String getHumi() {
  return String(carrier.Env.readHumidity());
}

// get light from IOT Carrier
void getLight() {  
  while (! carrier.Light.colorAvailable()) {
    delay(5);
  }
  carrier.Light.readColor(r, g, b, light);
}

// get humidity from SIL sensor
String getSoilHum() {
  int raw_moisture = analogRead(moistPin);
  return String(map(raw_moisture, 0, 400, 0, 100));
}

// format a measurement and its value to line protocol format
String formatLineProtocol(String measurement, String value){
  String result = measurement;
  result += ",device=" + deviceUID + ",location=" + location + " value=" + value + "";
  return result;
}

// Activate or deactivate the fan
void fanCtrl(int val) {
  analogWrite(fanPIN, val);
}