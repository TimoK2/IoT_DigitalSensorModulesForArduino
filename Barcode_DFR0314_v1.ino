/*
This example shows how to connect a barcode reader DFR0314 to Arduino board serial interface.
The reader DFR0314 is similar to other compact bar code reader modules. The documentation:
https://wiki.dfrobot.com/Barcode_Reader_Scanner_Module-CCD_Camera_SKU_DFR0314

In the barcode reader the serial connection is the traditional RS232 with +/-12 V signal levels.

In the Arduino MKR1000, MKR Zero, etc. the serial interface output TX and input RX are with 
0V/3,3V signal levels.

In between a serial coverter is needed. The ICL3232CPZ from Renesas is used.
https://www.renesas.com/us/en/products/interface/rs-485-rs-422-rs-232/rs-232/device/ICL3232.html

The connection is like in the data sheet the connection "Typical operating circuits ICL3232".
https://www.renesas.com/us/en/www/doc/datasheet/icl3221-22-23-32-41-43.pdf

MKR ------ ICL3232 ------ DFR0314

5V  --------------------- D9
VIN --5V...7V ( or connected to USB )
VCC ------- Vcc(16)
GND ------- GND(15)--GND
                     GND- D5
TX  ------- T1IN(11)
            T1OUT(14)---- D3
            R1IN(13) ---- D2
RX  ------- R1OUT(12)
      GND-- T2IN(10) 
      GND-- R2IN(8)
    GND-C5- Vcc(16)
    GND-C3- V+(2)
    GND-C4- V-(6)
        C1- C1+(1)
        C1- C1-(3)
        C2- C2+(4)
        C2- C2-(5)

Timo Karppinen 18.12.2018
  
*/
String code = "";           //initialize the output string
boolean endBit = 0;            //a flag to mark 0D received
char temp;
int ledPin = LED_BUILTIN;

void setup() {
  Serial.begin(9600);       //initialize the Serial port
  Serial1.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (Serial1.available() > 0)     {
    temp = char( Serial1.read());    //read the input data
    code += temp;
  }
  if (temp == 0x0D)   // Or temp == '\r'
  {          
    Serial.println(code);
    code = "";
    endBit = 0;
    temp = 0;
  }
  else
  {
    endBit = 1;
  }
  if (endBit == 0)
  {
  digitalWrite(ledPin, !endBit);   // reading is done!
  delay(1000);
  }
  digitalWrite(ledPin, LOW);
}
