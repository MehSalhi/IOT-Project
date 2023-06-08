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

#define SECRET_SSID "."
#define SECRET_PASS "hqh97n8ircpb5bj"

#define fanON   255
#define fanOFF  0
#define fanPIN  A3

MKRIoTCarrier carrier; 
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

String location = "serre_1";

const char broker[] = "192.168.9.194";
int        port     = 1883;

String topicSub  = "commander/"; // A tester sous forme de String
String topicPub  = "arduino";

String deviceUID = "";

const int moistPin = A6;
const int fanPin = 14;

// Sensors
#define tempIOTC
#define humiIOTC
#define lighIOTC
#define humiSOIL
#define movePIR

int r, g, b, light;
int pir;

IPAddress commander(192,168,9,194); //your central server address

//set interval for sending messages (milliseconds)
// TODO ajouter les intervals pour chaque capteur individuel
const long interval = 20000;
unsigned long previousMillis = 0;
int count = 0;

void setup() {
  // put your setup code here, to run once:
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

  pir = carrier.getBoardRevision() == 1 ? A5 : A0;
  carrier.display.setRotation(0);
  delay(1500);
  pinMode(pir, INPUT);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
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

  // set the message receive callback
  mqttClient.onMessage(listenTopics);

  Serial.print("Subscribing to topic: ");
  Serial.println(topicSub);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topicSub);
  
}

void loop() {
  // put your main code here, to run repeatedly:
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

void listenTopics(int messageSize){
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());      // TODO Changer pour effectuer les actions en fonction du topic
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();
  Serial.println();
}

void publishTopics(String topic, String value){
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(value);

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic);
  mqttClient.print(value);
  mqttClient.endMessage();
  delay(500);
}

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

  // counts the number of sensors
  unsigned int sensorCount = 0;  

  // append sensor data conditionnaly
  
  #ifdef tempIOTC
    //Serial.print("tempIOTC: ");
    ++sensorCount;
    String temp = getTemp();
    //Serial.println(temp);
    // convert and format to string
    publishTopics(topicPub, temp);
  #endif
  
  #ifdef humiIOTC
    //Serial.print("humiIOTC: ");
    ++sensorCount;
    String humi = getHumi();
    //Serial.println(humi);
    // convert and format to string
    publishTopics(topicPub, humi);
  #endif

  #ifdef lighIOTC
    //Serial.print("lighIOTC: ");
    ++sensorCount;
    
    String resLight = getLight();
    //Serial.println(light);
    // convert and format to string
    publishTopics(topicPub, resLight);
  #endif

  #ifdef humiSOIL
    //Serial.print("humiSOIL: ");
    ++sensorCount;
    String humiSoil = getSoilHum();
    //Serial.println(humiSoil);
    // convert and format to string
    publishTopics(topicPub, humiSoil);
  #endif

  #ifdef movePIR
    //Serial.print("movePIR: ");
    ++sensorCount;    
    int prox = getProx();
    //Serial.println(prox);
    // convert and format to string
    //data["prox"] = prox;
  #endif

  // TODO: move out of this func
  // should be used to display the return of getData()
  carrier.display.setCursor(0, 0);
  carrier.display.fillScreen(ST77XX_BLACK);
  //carrier.display.print("tempIOTC\n" + String(temp) + "\n" + "humiIOTC\n" + humi + "\n" + "lighIOTC\n" + light + "\n" + "humiSOIL\n" + humiSoil + "\n" + "movePIR\n" + prox);
  
}

// package data to send

// send data

// get temperature from IOT Carrier
String getTemp() {
  float temp = carrier.Env.readTemperature();
  return formatLineProtocol("temperature", String(temp));
}

// get humidity from IOT Carrier
String getHumi() {
  return formatLineProtocol("humidity", String(carrier.Env.readHumidity())) ;
}

// get light from IOT Carrier
String getLight() {  
  while (! carrier.Light.colorAvailable()) {
    delay(5);
  }
  return formatLineProtocol("light", String(carrier.Light.readColor(r, g, b, light)));
}

// get humidity from SIL sensor
String getSoilHum() {
  int raw_moisture = analogRead(moistPin);
  return formatLineProtocol("soil humidity", String(map(raw_moisture, 0, 1023, 0, 100)));
}

// get movement from PIR sensor
int getProx() {
  return digitalRead(pir);
}

String formatLineProtocol(String measurement, String value){
  String result = measurement;
  result += ",device=\"" + deviceUID + "\",location=\"" + location + "\" value=\"" + value + "\"";
  return result;
}

void fanCtrl(int val) {
  analogWrite(fanPIN, val);
}