/*
This program reads the raw data from a digital 3 axis accelerometer sensor

Arduino hardware can be for example 
Nano:  the pins for I2C communication are A4 = SDA data and A5 = SCL clock
Uno:   A4 = SDA data and A5 = SCL clock
MKR1000: 11 = SDA data and 12 = SCL clock

Accelerometer sensor is MMA7660FC manufactured by NXP
http://www.nxp.com/assets/documents/data/en/data-sheets/MMA7660FC.pdf

The sensor is mounted on sensor board Grove - 3-Axis Digital Accelerometer(±1.5g)
SKU 101020039
https://www.seeedstudio.com/Grove-3-Axis-Digital-Accelerometer%28%C2%B11.5g%29-p-765.html

The communication over I2C bus is documented in MMA7660FC data sheet. The library
making Arduino run the I2C communication is documented in Wire class reference
at Arduino.cc

The sensor 6 bit two's complement is converted to 16 bit integer. 16 bit integer
is a two's complement by definition and the print function prints it correctly
as decimal number.

Please note that in MKR1000 and all the other SAMD processor Arduinos the integers are 32 bit !
So we work with 32 bit integers in two's complement !

Timo Karppinen
24.1.2017
32 bit for MKR1000 9.9.2019
to 7 segment numbers 11.9.2019
 */

#include <Wire.h>

#define sensorAddr 0x4C //factory set slave address for the MMA7660

signed int x_out, y_out, z_out;  // values to print out as 16 bit or 32 integer, two's complement
signed int x_out_6bit, y_out_6bit, z_out_6bit;  // values to print out with 6 bit two's complement accuracy

float T;
float angleXZ = 0;
int angleXZint = 0;

// for the 7 segment display
int ledPin = LED_BUILTIN;
int ledState = LOW;

int switchAPin = 4;
int switchAState = LOW;

int switchBPin = 5;
int switchBState = LOW;

int dataPin = 8;
int clockPin = 9;
int registerClockPin = 3;

int i = 0;
int j = 0;

const int seqNumbers[10] = {B11111100 ,B01100000 ,B11011010, B11110010, B01100110, B10110110, B10111110, B11100000, B11111110, B11110110 };


void setup()
{
  // Intitialize the serial port
  Serial.begin(9600);

 // Initialize the 3 axis accelerometer
  Wire.begin();       //Initialize the Wire library and join the I2C bus as a master
                      // in Nano the pins are Analog4 = SDA, Analog5 = SCL
  delay(500);
  sendI2C(0x07,0x00);  
    // (0x07) = register 7 Mode
    // (0x00) = Mode set to Stand by

  sendI2C(0x06,0x10);
    // (0x06) = register 6 Interrupt Setup 
    // (0x10) = interrupt after every measurement


  sendI2C(0x08,0x00);
   // (0x08) = register 8 Sample Rate 
   // (0x00) = 120 samples in second in active and auto sleep mode, detects
            // tabs, filters results and maitains rolling average
            // on x, y and Z. Produces filtered values every
            // 8,3 millisecond
                    
  sendI2C(0x07,0x01);                  
   // (0x079 =  register 7 Mode
   // (0x01) = Mode set to Start

// for the 7segment
  pinMode(ledPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(registerClockPin, OUTPUT);

  delay(500);
}
void sendI2C(unsigned char registerAddr, unsigned char registerData)
{
  Wire.beginTransmission(sensorAddr);
  Wire.write(registerAddr);
  Wire.write(registerData);
  Wire.endTransmission();
}

void convert32bit(unsigned int x_in, unsigned int y_in, unsigned int z_in)
//void convert16bit(int x_in, int y_in, int z_in)
{
signed int x_32;
signed int y_32;
signed int z_32;
x_32 = x_in << 26;    // shift left the get the sign bit as leftmost bit
y_32 = y_in << 26;
z_32 = z_in << 26;

x_out = x_32;   // the value. This time no mathematics
y_out = y_32;   // 16 bit two's complement -2 147 483 648  .... 2 147 483 647
z_out = z_32;   // 9,81 m/s2 = 1 g 
                //  1 g =  1 431 655 764‬ = 0101 0101 0101 0101 0101 0101 0101 0100
                // -1 g = -1 431 655 764‬ = 1010 1010 1010 1010 1010 1010 1010 1100

if ( z_out >= x_out)
 {
  T = ((float)x_out)/((float)z_out);                              // arctan
  angleXZ = (180/3.1416)*(T - 0.3333*(T*T*T) + 0.2*(T*T*T*T*T));  // series expansion
 }                                                               // for angles < 45 degrees

if ( z_out < x_out )
 {
  T = ((float)z_out)/((float)x_out);
   angleXZ = 90 - (180/3.1416)*(T - 0.3333*(T*T*T) + 0.2*(T*T*T*T*T));
 }
                
}


void loop()
{
  unsigned int x,y,z;
  delay(1000); // There will be new values every 1000ms
               // or delay(10) every 10 ms

  Wire.beginTransmission(sensorAddr);
  Wire.write(0x00);  // register to read
  Wire.endTransmission();

  Wire.requestFrom(sensorAddr, 3); 
  if (Wire.available() )
  {
    x=Wire.read(); //  sign + 5 bit , two's complement
    y=Wire.read(); //  look at wikipedia.org for two's complement
    z=Wire.read(); //  In finnish: kahden komplementti
  }

  convert32bit(x,y,z);

  Serial.print("x:         ");
  Serial.print(x);
  Serial.print(" y:        ");
  Serial.print(y);
  Serial.print(" z:        ");
  Serial.println(z);
  
  Serial.print("x: ");
  Serial.print(x_out);
  Serial.print(" y: ");
  Serial.print(y_out);
  Serial.print(" z: ");
  Serial.println(z_out);

  x_out_6bit = x_out/67108864;
  y_out_6bit = y_out/67108864;
  z_out_6bit = z_out/67108864;

  Serial.print("x:         ");
  Serial.print(x_out_6bit);
  Serial.print(" y:        ");
  Serial.print(y_out_6bit);
  Serial.print(" z:        ");
  Serial.print(z_out_6bit);
  Serial.print(" angle XZ: ");
  Serial.println(angleXZ);

 // develop a code here which is converting the float angleXZ into numbers i and j !
 // i = 1;
 // j = 2;  

 angleXZint = int(angleXZ);   // converting to integer number
 angleXZint = abs(angleXZint);  // taking the absolute value
 i = angleXZint/10;
 j = angleXZint%10;
 
   digitalWrite(registerClockPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, seqNumbers[j]);
   shiftOut(dataPin, clockPin, LSBFIRST, seqNumbers[i]);
   digitalWrite(registerClockPin, HIGH);
}
