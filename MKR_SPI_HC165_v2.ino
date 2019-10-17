/*
MKR1000 connecting to YYYYYYYYYY MQTT Broker
or to IBM Cloud Watson IoT

Timo Karppinen 19.2.2017

Modified for testing SPI and 74HC165 parallel in serial out shift register.
The parallel connection in each 74HC165 is used for connecting 8 sensor False/True signals.
https://www.onsemi.com/pub/Collateral/MC74HC165A-D.PDF
 
Please connect
MKR1000 - 74HC165
GND - 5 GND
Vcc - 6 Vcc
9 SCK - 2 Serial clock
10 MISO - 9 Serial output
1 -  1 Parallel load
0 -  15 SPICS   chip select

Timo Karppinen 16.4.2019

 */

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiSSLClient.h>
#include <MQTTClient.h>

// WLAN 
char ssid[] = "Moto_Z2_TK"; //  your network SSID (name)
char pass[] = "xxxxxxxxxxx";    // your network password (use for WPA)

//char ssid[] = "HAMKvisitor"; //  your network SSID (name)
//char pass[] = "xxxxxxxxxxx";    // your network password (use for WPA)

//char ssid[] = "Nelli";
//char pass[] = "xxxxxxxxxxx";


//char *client_id = "<some characters making this unique from mqtt broker point of view>"; 
char *client_id = "A_MKR1000_F3AC";  
//char *user_id = "xxxxxxxxxx";   // user name, if required on mqtt broker
//char *authToken = "xxxxxxxxx"; // password, if required on mqtt broker

//char *mqtt_hostname = “mqtt broker name or ip address”;
char *mqtt_hostname = "yyy.yyyyy.yyyy.fi";

// sensors and LEDS
const int LEDPin = LED_BUILTIN;     // must be a pin that supports PWM. 0...8 on MKR1000
// HC165
const int SPICS = 0;    // chip select for sensor SPI communication
const int hc165PLoad = 1;    // parallel load on HC165    
byte hc165Byte0 = 0;           // 8 bit data from sensor board
byte hc165Byte1 = 0;           // 8 bit data from sensor board

int hc165raw0 = 0;
int hc165raw1 = 0;
int hc165raw = 0;

/*use this class if you connect using SSL
 * WiFiSSLClient net;
*/
WiFiClient net;
MQTTClient MQTTc;

unsigned long lastSampleMillis = 0;
unsigned long previousWiFiBeginMillis = 0;
unsigned long lastMQTTMillis = 0;
unsigned long lastPrintMillis = 0;


void setup() 
{
  Serial.begin(9600);
  delay(2000); // Wait for wifi unit to power up
  WiFi.begin(ssid, pass);
  delay(5000); // Wait for WiFi to connect
  Serial.println("Connected to WLAN");
  printWiFiStatus();
  
  /*
    client.begin("<Address Watson IOT>", 1883, net);
    Address Watson IOT: <WatsonIOTOrganizationID>.messaging.internetofthings.ibmcloud.com
    Example:
    client.begin("iqwckl.messaging.internetofthings.ibmcloud.com", 1883, net);
  */
  MQTTc.begin(mqtt_hostname, 1883, net);  // Cut for testing without Watson

  connect();

  SPI.begin();
  // Set up the I/O pins
  pinMode(SPICS, OUTPUT);
  pinMode(hc165PLoad, OUTPUT);

  digitalWrite(SPICS, HIGH);  // for not communicating with SPI at the moment
  digitalWrite(hc165PLoad, LOW); // for loading inputs to parallel register
  
 }

void loop() {
   MQTTc.loop();  // Needs to be here in the loop(). Don't use any
                  // longer delays with delay() in the loop!
                  // Otherwise the mqtt broker breaks the connection. 
 

  // opening and closing SPI communication for reading sensor
  if(millis() - lastSampleMillis > 500)
  { 
    lastSampleMillis = millis();
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    
    digitalWrite(hc165PLoad, HIGH);
    digitalWrite(SPICS, LOW);
  
    hc165Byte0 = SPI.transfer(0x00);
    hc165Byte1 = SPI.transfer(0x00);

 
    digitalWrite(SPICS, HIGH);
    digitalWrite(hc165PLoad, LOW);
    SPI.endTransaction();

    hc165raw0 = hc165Byte0 << 8;
    hc165raw1 = hc165Byte1;
    hc165raw =( hc165raw0 | hc165raw1 );  // Bitwise OR operation
    
  }

  // Print on serial monitor once in 1000 millisecond
  if(millis() - lastPrintMillis > 1000)
  {
    Serial.print("als bytes from sensor   ");
    Serial.print(hc165Byte0, BIN);
    Serial.print("  ");
    Serial.print(hc165Byte1, BIN);
    Serial.print("  2 sensors   ");
    Serial.println(hc165raw, BIN);
    
 
    lastPrintMillis = millis();
  }
  
     // publish a message every  20 second.
     if(millis() - lastMQTTMillis > 20000) 
     {
      
      Serial.println("Publishing to mqtt Broker...");
        if(!MQTTc.connected()) {    // Cut for testing without mqtt
         connect();                 // Cut for testing without mqtt
        }                           // Cut for testing without mqtt
        lastMQTTMillis = millis();
         //Cut for testing without mqtt
    
          String wpayload = "{\"Installation\":\"Test\",\"Sensor type \":\"2xhc165 \",\"Sensor values in binary\":" + String(hc165raw)+ "}";
           
          MQTTc.publish("hamk/ICT_R/Ri-KA-A103", wpayload);
     }
   
    delay(1);
    
// end of loop
}

void connect() 
{
  Serial.print("checking WLAN...");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");       // printing a dot every half second
    if ( millis() - previousWiFiBeginMillis > 5000) // reconnecting
    {
      previousWiFiBeginMillis = millis();
      WiFi.begin(ssid, pass);
      delay(5000); // Wait for WiFi to connect
      Serial.println("Connected to WLAN");
      printWiFiStatus();
    }
    delay(500);
    
  }
  /*
    Example:
    MQTTc.connect("d:iqwckl:arduino:oxigenarbpm","use-token-auth","90wT2?a*1WAMVJStb1")
    
    Documentation: 
    https://console.ng.bluemix.net/docs/services/IoT/iotplatform_task.html#iotplatform_task
  */
  
  Serial.print("\nconnecting MQTT Broker with MQTT....");
  // Cut for testing without Broker
//while (!MQTTc.connect(client_id,user_id,authToken)) 
while (!MQTTc.connect(client_id)) 
  {
    Serial.print(".");
    delay(30000); // try again in thirty seconds
  }
  Serial.println("\nconnected!");
}

// messageReceived subroutine needs to be here. MQTT client is calling it.
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
