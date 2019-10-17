/************************************************************************
*
* Test of the Pmod OLEDRGB display module with Pmod ALS ambient light sensor
* https://store.digilentinc.com/pmod-oledrgb-96-x-64-rgb-oled-display-with-16-bit-color-resolution/
* https://store.digilentinc.com/pmod-als-ambient-light-sensor/
*************************************************************************
* Pmod_OLEDRGB
* The message "Test module Pmod Digilent Lextronic" will be display on OLEDrgb module
* with different size and colors
*
* Material
* 1. Arduino MKR1000
* 2. Pmod OLEDrgb (Download libraries
* https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
* https://github.com/adafruit/Adafruit-GFX-Library)
* There is no need to install these from the Github. In Arduino IDE go to "Include Libraries".
* OLED pins
* OLED 4 - SCK - MKR1000 9  hardware defined for the SPI
* OLED 2 -MOSI- MKR1000 8  hardware definef for the SPI
* OLED 1 - CS - MKR1000 2  or any other free
* OLED 7 - DC - MKR1000 3  or any other free
* OLED 8 -RES - MKR1000 4  or any other free
* OLED 9 - VCC Enable - VCC
* OLED10 - Pmod Enable - VCC
*************************************************************************
* Pmod ALS ambient light sensor
* Please connect
* MKR1000 - PmodALS
* GND - 5 GND
* Vcc - 6 Vcc
* 9 SCK - 4 SCK
* 10 MISO - 3 MISO
* 1 - 1 SS

ALS data on the SPI
D15 ... D13 - three zeros
D12 ... D04 - 8 bits of ambient light data
D03 ... D00 - four zeros

Details on the connection of ADC chip on ALS board are given
http://www.ti.com/lit/ds/symlink/adc081s021.pdf
* 
* The ambient light value will be displayed. Please try to calibrate it
* with a hight quality luxmeter.
* Timo Karppinen 8.10.2019
**************************************************************/

// Call libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>


//static int sck = 9;  // this line needed if software SPI will be used
//static int mosi = 8; // this line needed if software SPI will be used
static int cs = 2;
static int dc = 3;
static int res = 4;

// Definition of colors
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
Adafruit_SSD1331 OLED = Adafruit_SSD1331(cs, dc, res); // three parameters = with hardware SPI
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
 OLED.fillRoundRect(35, 5, 30, 40, 5, Blue);
 OLED.fillRoundRect(65, 5, 30, 40, 5, Red);
 OLED.fillCircle(90, 55, 5, Yellow); // yellow circle with radius=5 in=90 and y=55
 delay(2000); // wait 2 s
}

 if(millis() - lastSampleMillis > 500)
  { 
    lastSampleMillis = millis();
    SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    digitalWrite(alsCS, LOW);
  
    alsByte0 = SPI.transfer(0x00);  // executing SPI transfer for 8 bits out and in
                                    // reading 8 bits from SPI register to variable alsByte0
    alsByte1 = SPI.transfer(0x00);  // executing SPI transfer for 8 bits out and in
                                    // reading 8 bits from SPI register to variable alsByte1  
    digitalWrite(alsCS, HIGH);
    SPI.endTransaction();

                                    // ALSByte0 and ALSByte1 contain now
                                    // nnnnHHHH and LLLLnnnn where
                                    // H means ambient light value higher bits
                                    // L means ambient light value lower bits
    alsByteSh0 = alsByte0 << 4;
    alsByteSh1 = alsByte1 >> 4;
    
    als8bit =( alsByteSh0 | alsByteSh1 );  // now there should be the correct bits in correct place
                                           // Ambient light value as HHHHLLLL 8 bit number.
    alsRaw = als8bit; // 
    alsScaledF = float(alsRaw)*6.68;  // calibrate with "y=ax+b" style function!
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
