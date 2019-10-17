/*

Connecting a thermo couple sensor with SPI on a MKR1000 and sending 
measurement data over the UDP

Connecting SPI thermocouple board Digilent PmodTC1
Please connect
MKR1000 - PmodTC1

GND - 5 GND
Vcc - 6 Vcc
9 SCK - 4 SCK
10 MISO - 3 MISO
3 - 1 SS

Thermocouple data on the SPI
D31 - sign
D30 ....D18 - 13 bits of temperature data
D16 - normally FALSE. TRUE if thermocouple input is open or shorted to GND or VCC
D15 ... D0 - reference junction temperature

The reference junction compensation is calculted in the IC. No need to calculate here.

Please send one UDP message from the UDP server first. That is the way of getting the IP addres
for sending back the measurement values. 

5.9.2019
Timo Karppinen
 */


#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

int status = WL_IDLE_STATUS;

// WLAN 
char ssid[] = "Moto_Z2_TK"; //  your network SSID (name)
char pass[] = "4a830c45838e";    // your network password (use for WPA)

//char ssid[] = "HAMKvisitor"; //  your network SSID (name)
//char pass[] = "hamkvisitor";    // your network password (use for WPA)

//char ssid[] = "Nelli";
//char pass[] = "xxxxxxxxx";


// UDP 
unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged by ";       // a string to send back

WiFiUDP Udp;

// sensors and LEDS
const int LEDPin = LED_BUILTIN;     // must be a pin that supports PWM. 0...8 on MKR1000
// PModTC1
const int thermoCS = 3;         // chip select for MIC3 SPI communication
int thermoByte0 = 0;           // 8 bit data from TC1 board
int thermoByte1 = 0;           // 8 bit data from TC1 board
int thermoByte2 = 0;           // 8 bit data from TC1 board
int thermoByte3 = 0;           // 8 bit data from TC1 board
int temp14bit = 0;             // 14 most significant bits on a 32 bit integer
int tempRaw = 0;
float tempScaledF = 0;

int blinkState = 0;

// Timing of events
unsigned long lastSampleMillis = 0;
unsigned long previousWiFiBeginMillis = 0;
unsigned long lastUDPMillis = 0;
unsigned long lastPrintMillis = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

// Starting WLAN
 connect();

// Starts UDP and starts waiting for the first UDP message
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

// Set up the I/O pins
  SPI.begin();
  
  pinMode(thermoCS, OUTPUT);
  pinMode(LEDPin, OUTPUT);

// Updating timers after making succesfull connection
  lastSampleMillis = millis();
  lastUDPMillis = millis();

}

void loop() {

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);


  // Send a reply, to the IP address and port that sent us the packet we received
  //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());   //UDP port picked up from the received message
    Udp.beginPacket(Udp.remoteIP(), 8888);  // UDP port given as a number
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }

 // opening and closing SPI communication for reading TC1
  if(millis() - lastSampleMillis > 5000)
  { 
    lastSampleMillis = millis();
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    digitalWrite(thermoCS, LOW);
  
    thermoByte0 = SPI.transfer(0x00);
    thermoByte1 = SPI.transfer(0x00);
    thermoByte2 = SPI.transfer(0x00);
    thermoByte3 = SPI.transfer(0x00);
  
    digitalWrite(thermoCS, HIGH);
    SPI.endTransaction();


    thermoByte0 = thermoByte0 << 24;
    thermoByte1 = thermoByte1 << 16;
    
    
    temp14bit =( thermoByte0 | thermoByte1 );
    
    tempRaw = temp14bit/262144; // shifting 18 bits to right gives multiply of 0,25 degree C.
    tempScaledF = float(temp14bit/262144)/4;
    //tempScaledF = 15.678;


  }
 // Print on serial monitor once in 1000 millisecond
  if(millis() - lastPrintMillis > 1000)
  {
    Serial.print("temp14bit   ");
    Serial.println(temp14bit, BIN);
    Serial.print("  tempRaw  ");
    Serial.println(tempRaw, BIN);
    Serial.print("  tempScaledF  ");
    Serial.println(tempScaledF);
 
    lastPrintMillis = millis();
  }



 // publish a message every  10 second.
  if(millis() - lastUDPMillis > 10000) 
  {
    lastUDPMillis = millis();
    Serial.println("Publishing to UDP...");
  /*  if(WiFi.status() != WL_CONNECTED)
      {
        connect();                 
      }                        
   */
        
    String UDPpayload_object = "Temp scaled " + String(tempScaledF,2);
    char Reply2_0Buffer[] = "measurements";
    char Reply2_1Buffer[] = "";
    UDPpayload_object.toCharArray(Reply2_1Buffer,17); // String object converted to character array
    
    //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());   //UDP port picked up from the received message
    Udp.beginPacket(Udp.remoteIP(), 8888);  // UDP port given as a number
    Udp.write(Reply2_0Buffer);
    Udp.endPacket();
    delay(1000);
    Udp.beginPacket(Udp.remoteIP(), 8888);  // UDP port given as a number
    Udp.write(Reply2_1Buffer,17);
    Udp.endPacket();

  }
delay(100);  
}   // end of loop

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
}

void printWiFiStatus() 
{
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
