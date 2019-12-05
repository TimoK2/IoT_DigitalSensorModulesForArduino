/************************************************************************
*
* Test of the Pmod small OLED display and ambient light sensor
*
*************************************************************************
* Description: Pmod_OLEDrgb
* The message "Test module Pmod Digilent Lextronic" will be display on OLEDrgb module
* with different size and colors..... and sensor value as well.
*
* Material
* 1. Arduino MKR1000
* 2. Pmod OLEDrgb  libraries
* https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
* https://github.com/adafruit/Adafruit-GFX-Library)
* These libraries can be found with the Arduino Library Manager
* 
* Please connect MKR1000 - Pmod_OLEDrgb with lines:
* MKR1000 9 - OLED 4 SCK   hardware defined for the SPI
* MKR1000 8 - OLED 2 MOSI  hardware definef for the SPI
* MKR1000 2 - OLED 1 CS   or any other free
* MKR1000 3 - OLED 7 DC   or any other free
* MKR1000 4 - OLED 8 RES   or any other free
*  VCC      - OLED 9 Enable - VCC
*  VCC      - OLED10 Pmod Enable - VCC
*  GND      - OLED 5 Ground
*  VCC      - OLED 6 Power supply 3,3 V
* 
*************************************************************************
* Pmod ALS ambient light sensor
* Please connect MKR1000 - PmodALS with lines:
* MKR1000 9  - ALS 4 SCK
* MKR1000 10 - ALS 3 MISO
* MKR1000 1  - ALS 1 SS
* GND        - ALS 5 GND
* Vcc        - ALS 6 Vcc

ALS data on the SPI
D15 ... D13 - three zeros
D12 ... D04 - 8 bits of ambient light data
D03 ... D00 - four zeros

Details on the connection of ADC IC chip on ALS board are given
http://www.ti.com/lit/ds/symlink/adc081s021.pdf
* 
* Timo Karppinen 5.12.2019
**************************************************************/

// Call libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>


//const int sck = 9;  // harware SPI: no need to specify in code
//const int mosi = 8; // hardware SPI: no need to specify in code
const int OLEDcs = 2;
const int dc = 3;
const int res = 4;

// Definition of colors on the OLED display
#define Black 0x0000
#define Blue 0x001F
#define Red 0xF800
#define Green 0x07E0
#define Cyan 0x07FF
#define Magenta 0xF81F
#define Yellow 0xFFE0
#define White 0xFFFF


// PModALS
const int alsCS = 1;        // chip select for sensor SPI communication
byte alsByte0 = 0;           // 8 bit data from sensor board
byte alsByte1 = 0;           // 8 bit data from sensor board
byte alsByteSh0 = 0;
byte alsByteSh1 = 0;
int als8bit = 0;
int alsRaw = 0;
float alsScaledF = 0;

int first = 0;
unsigned lastSampleMillis = 0; 
unsigned lastPrintMillis = 0; 

//Adafruit_SSD1331 OLED = Adafruit_SSD1331(cs, dc, mosi, sck, res); // five parameters =with software SPI
Adafruit_SSD1331 OLED = Adafruit_SSD1331(OLEDcs, dc, res); // three parameters = with hardware SPI
// The library class includes the call SPI.begin !

void setup(void)
{
  Serial.begin(9600);
  pinMode(alsCS, OUTPUT);
  digitalWrite(alsCS, HIGH);   // for not communicating with ALS at the moment


 OLED.begin(); // initialization of display objcet
}

void loop()
{
while(first < 3)
{ 
 first += 1; 
 OLED.fillScreen(Black); // background screen in black
 OLED.setTextColor(Cyan); // color of text in cyan
 OLED.setCursor(0,0); // cursor is in x=0 and y=15
 OLED.print("Test module Pmod"); // display text
 delay(500); // wait 500 ms
 OLED.setCursor(0,15); // cursor is in x=0 and y=15
 OLED.setTextSize(2); // size of text
 OLED.setTextColor(Red); // text in red color
 OLED.println("DIGILENT"); // display text
 OLED.setCursor(20,40); // cursor is in x=20 and y=40
 OLED.setTextSize(1); // size of text
 OLED.setTextColor(Green); // text in green color
 OLED.println("LEXTRONIC"); // display text
 OLED.drawFastHLine(1, 60, OLED.width()-1, Blue); // blue line x=1 to screen width-1 and y=60
 delay(2000); // wait 2 s
 OLED.fillScreen(Black); // background display in black (erase display)
 OLED.fillRoundRect(5, 5, 30, 40, 5, Blue); // French flag bleu blanc rouge
 OLED.fillRoundRect(35, 5, 30, 40, 5, White);
 OLED.fillRoundRect(65, 5, 30, 40, 5, Red);
 OLED.fillCircle(90, 55, 5, Yellow); // yellow circle with radius=5 in=90 and y=55
 delay(2000); // wait 2 s
}

 if(millis() - lastSampleMillis > 500)
  { 
    lastSampleMillis = millis();
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    digitalWrite(alsCS, LOW);
  
    alsByte0 = SPI.transfer(0x00);
    alsByte1 = SPI.transfer(0x00);
  
    digitalWrite(alsCS, HIGH);
    SPI.endTransaction();


    alsByteSh0 = alsByte0 << 4;
    alsByteSh1 = alsByte1 >> 4;
    
    
    als8bit =( alsByteSh0 | alsByteSh1 );
    
    alsRaw = als8bit; // 
    alsScaledF = float(alsRaw)*6.68;
    Serial.print("Ambient light scaled =  ");
    Serial.println(alsScaledF);

  }

 if(millis() - lastPrintMillis > 2000)
 {
  lastPrintMillis = millis();
  OLED.fillScreen(Black); // background screen in black
  OLED.setTextColor(Cyan); // color of text in cyan
  OLED.setCursor(0,0); // cursor is in x=0 and y=15
  OLED.print("ALS lux = "); // display text
  OLED.println(alsScaledF);
  OLED.println("Test completed");
  Serial.println("printed on OLED");
  delay(400);
 }
  
}
