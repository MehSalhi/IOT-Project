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

#define SECRET_SSID "Ancalagon"
#define SECRET_PASS "actuelle"

MKRIoTCarrier carrier; 
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status
WiFiServer server(80);
WiFiClient initClient;

const int moistPin = A6;

// Sensors
#define tempIOTC
#define humiIOTC
#define lighIOTC
#define humiSOIL
#define movePIR

IPAddress commander(192,168,45,202); //your central server address

void setup() {
  // put your setup code here, to run once:
    //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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
  // if you get a connection, report back via serial:
  if (initClient.connect(commander, 8181)) {
    IPAddress localIp = WiFi.localIP();
    byte mac[6];
    WiFi.macAddress(mac);
    String tags = "[\"serre 1\"]";
    String capteurs = "[\"temp1\", \"hum1\"]";
    String payload = "{\"DevID\": " + String((char*)mac) + ", \"IpAddr\": " + localIp  + ", \"Tags\": " + tags + + ", \"Capteurs\": " + capteurs + "}";
    Serial.println("connected to server");
    // Make a HTTP request:
    initClient.println("POST /search?q=arduino HTTP/1.1");
    initClient.println("Content-Type: application/json");
    initClient.println("Host: 192.168.45.202:8181");
    initClient.println("Connection: close");
    initClient.println();
    initClient.println(payload); //Pass the parameters here
  }
  initClient.stop();
  server.begin();
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  printWifiStatus();
}

void loop() {
  // put your main code here, to run repeatedly:
  listenClients();
}

void listenClients(){
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 20");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          
          // Send the datas here
          client.println("You're at the root");
          
          client.println("<br />");
          
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          currentLine = "";
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
          currentLine += c;
        }

        if (currentLine.endsWith("GET /test")) {
               // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 20");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // Send the datas here
          client.print("My test");
          client.println("<br />");
          
          client.println("</html>");
          break;
        }        

      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
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

String getData() {
  // data that will be formatted and sent to the server
  String data = "";

  // counts the number of sensors
  unsigned int sensorCount = 0;  
  
  // append sensor data conditionnaly

  #ifdef tempIOTC
    ++sensorCount;
    float temp = getTemp();
    // convert and format to string
  #endif

  #ifdef humiIOTC
    ++sensorCount;
    float humi = getHumi();
    // convert and format to string
  #endif

  #ifdef lighIOTC
    ++sensorCount;
    int r, g, b, light;
    getLight(r, g, b, light);
    // convert and format to string
  #endif

  #ifdef humiSOIL
    ++sensorCount;
    //TODO
    // convert and format to string
  #endif

  #ifdef movePIR
    ++sensorCount;    
    int prox = getProx();
    // convert and format to string
  #endif
}

// package data to send

// send data

// get temperature from IOT Carrier
float getTemp() {
  return carrier.Env.readTemperature();
}

// get humidity from IOT Carrier
float getHumi() {
  return carrier.Env.readHumidity();
}

// get light from IOT Carrier
void getLight(int r, int g, int b, int light) {  
  carrier.Light.readColor(r,g, b, light);
}

// get humidity from SIL sensor
int getSoilHum() {
  int raw_moisture = analogRead(moistPin);
  return map(raw_moisture, 0, 1023, 0, 100);
}


// get movement from PIR sensor
int getProx() {
  return carrier.Light.readProximity();
}

