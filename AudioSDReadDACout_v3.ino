/*
Reading audio file from SD card and playing out from DAC output A0. Repeats
the audio file in a loop. Each audio 16 bit sample can be modofied individually.

In digital filtering out sample is calculated based on previous in samples and
out samples. If interested the key word for search is DSP.

 The circuit:
 * SD card attached to SPI bus as follows:
 ** VCC  - MKR 3.3 V - digilent pmodSD 6
 ** GND  - MKR GND - digilent pmodSD 5 
 ** CLK - MKR pin 9 - digilent pmodSD 4
 ** MISO - MKR pin 10 - digilent pmodSD 3
 ** MOSI - MKR pin 8 - digilent pmodSD 2
 ** CS - MKR1000 where you connected Cs (for MKRZero SD: SDCARD_SS_PIN) - digilent pmodSD 1

 * Audio amplifier board as follows:
 ** VCC - 3,3 volt regulated power or 5,0 volt regulated power for about one Ampere
 ** GND - MKR GND
 ** in + - MKR A0
 ** in - - MKR GND

Based on Arduino examples SD - ReadWrite, AudioZero - SimpleAudioPlayerZero

version 2: 
Removed all Serial.print() and Serial.write() inside loop to get speed up.
Timing with micros()

Good starting point is to produce a wav file with an online signal generator
for example http://www.wavtones.com/functiongenerator.php  with settings:
Waveform  Sine
Frequency 400 Hz
Level -6 or - 3 dBFS
Duration 3 s
Sampling rate 8 kHz
You will get 20 samples on each sine wave. The same file you can play with different 
sampling rate settings.

Timo Karppinen 2017
This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>

File myFile;

char c2 = 0;
char c1 = 0;
int i1 = 0;
int i2 = 0;
int sound16bit=0;
int sound32bit=0;
int sound10bit = 0;
unsigned long previousMicros = 0;
const long interval = 1000000;        // fs = 8000 hz -> 125 micro secs, fs = 24 000 Hz -> 41 micro secs
                                 // fs = 48 kHz -> 21 micro secs
                                 // fs = 1000000 -> 1 sec   Comment all "Serial.print(...); lines from 
                                 //                         "while(myFile.available())" loop for faster 
                                 //                         sampling rates
unsigned long currentMicros = micros();

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(3000);  // wait for opening serial monitor
 
  Serial.print("Initializing SD card...");
  if (!SD.begin(1))      // where you connected SD card chip select CS
  {
    Serial.println("initialization failed!");
    while(true);
  }
  Serial.println("initialization done.");

   analogWriteResolution(10); 
}

void loop() {
  // re-open the file for reading:
    Serial.println("opening file");
  myFile = SD.open("audio001.wav");
  if (myFile) 
  {
    Serial.println("audio.wav:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) // repeats itself until end of file
    {
      // Reading sample and generating audio out sample
      currentMicros = micros();
      if( currentMicros - previousMicros > interval )
      {
      previousMicros = micros();
      c1 = myFile.read();
      c2 = myFile.read();
      Serial.write(c1);
      Serial.print("   ");
      Serial.write(c2);
      Serial.print("   ");

      i2 = int(c1);
      i1 = int(c2);
      
      Serial.print(i1);
      Serial.print("  ");
      Serial.print(i2);
      Serial.print("  ");

        
       i1 = i1 << 8;
       sound16bit = i1 | i2;
       Serial.print(sound16bit);
       Serial.print("  ");

       sound32bit = sound16bit << 16;
       Serial.print(sound32bit);
       Serial.print("  ");
       
       sound10bit = sound32bit / 2097152; // 2^22
       Serial.print(sound10bit);
       Serial.print("  ");

       /*
        Insert your DSP code here !
       */

       // For the MKR1000 or MKRZERO analog out AO the audio sample values
       // need to be positive integers between 0 and 1023
       sound10bit = sound10bit + 1024;
      Serial.print(sound10bit);

      sound10bit = sound10bit/10;
      Serial.println("  ");

     
      analogWrite(A0,sound10bit);
      }          // end of reading sample and generating audio out sample
    }            // looping to next 16 bit sample in wav file
  }
    else {
    // if the file didn't open, print an error:
    Serial.println("error opening audio.wav");
  }
    // close the file:
    myFile.close();

    delay(100);  // Increase the delay here if you would like hear a click in the 
                 // end of file
  
}  // end of loop()


