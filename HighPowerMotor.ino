/******************************************************************************
  Displays a text on the display scrolling from right to left
  
  bernhard@generationmake.de
  Original Creation Date: Dec. 21, 2019

  This code is Beerware; if you see me at the local, 
  and you've found our code helpful, please buy us a round!

  Hardware Connections:
  Connect DOGM132-5 to Adafruit Feather M0. Use Hardware SPI.
  SI    = 23 (Hardware SPI)
  SCLK  = 24 (Hardware SPI)
  CS    = 15
  A0    = 17
  RESET = 16
  Backlight (if needed) is connected via a transistor to pin 13)
  
  Distributed as-is; no warranty is given.
******************************************************************************/
#include <DogGraphicDisplay.h>
#include "ubuntumono_b_16.h"
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "ifx007t.h"
 
Adafruit_ADS1015 ads1015;    // Construct an ads1015 at the default address: 0x48
Ifx007t mot1;
Ifx007t mot2;

//use these defines for Arduino MKR WIFI 1010
//#define BACKLIGHTPIN A6
//#define EMERGENCYSTOP A4
//#define DIS_CS A1
//#define DIS_A0 A3
//#define DIS_RESET A2
//#define MOT1_1 2
//#define MOT1_2 3
//#define MOT1_EN 1
//#define MOT2_1 4
//#define MOT2_2 5
//#define MOT2_EN 0

//use these defines for Adafruit Feather M0
#define BACKLIGHTPIN 13
#define EMERGENCYSTOP 18
#define DIS_CS 15
#define DIS_A0 17
#define DIS_RESET 16
#define MOT1_1 10
#define MOT1_2 9
#define MOT1_EN 11
#define MOT2_1 6
#define MOT2_2 5
#define MOT2_EN 12
//

#define OFFSET 554
#define CUROFFSET1 112
#define CUROFFSET2 121

DogGraphicDisplay DOG;

void setup() {
  pinMode(EMERGENCYSTOP,  INPUT_PULLDOWN);   // set EMERGENCYSTOP pin to input with pull down
  pinMode(BACKLIGHTPIN,  OUTPUT);   // set backlight pin to output
  digitalWrite(BACKLIGHTPIN,  HIGH);  // enable backlight pin

  DOG.begin(DIS_CS,0,0,DIS_A0,DIS_RESET,DOGM132);   //CS = 15, 0,0= use Hardware SPI, A0 = 17, RESET = 16, EA DOGM132-5 (=132x32 dots)

  DOG.clear();  //clear whole display
   Wire.begin();
 
  Serial.begin(9600);
//  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\nI2C Scanner");
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
 
  delay(500);           // wait 0.5 seconds for next scan

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV)");
  ads1015.begin();
  mot1.begin(MOT1_1,MOT1_2,MOT1_EN);
  mot2.begin(MOT2_1,MOT2_2,MOT2_EN);

}

void loop() {
  static int counter=0;
  static int pwm=0;
  static int offset=DOG.display_width(); // static variable with the size of the display, so text starts at the right border
  int speed1, speed2, cur1, cur2;

  DOG.string(offset,2,UBUNTUMONO_B_16,"Hello to the scrolling World!"); // print "Hello World" in line 3 at position offset

  offset--; // decrasye offset so text moves to the left
  if(offset<-232) offset=DOG.display_width(); //our text is 232 pixels wide so restart at this value

  int emergencystop;
  int16_t adc0, adc1, adc2, adc3, adc;
  float vin;
    char buffer[50];

  emergencystop=digitalRead(EMERGENCYSTOP);
  adc0 = ads1015.readADC_SingleEnded(0);
  adc1 = ads1015.readADC_SingleEnded(1);
  mot1.disable();
  mot2.disable();
  delay(1);

  adc2 = ads1015.readADC_SingleEnded(2);
  adc3 = ads1015.readADC_SingleEnded(3);
  adc = analogRead(A0);
  vin=(float)adc/1024.0*3.3*16;

  counter++;
  if(counter<10)
  {
    mot1.stop();
    mot2.stop();
    pwm=0;
  }
  if(counter>10&&counter<256)
  {
//    pwm++;
    pwm+=5;
    mot1.pwm(pwm);
    mot2.pwm(pwm);
  }
  if(counter>256&&counter<768)
  {
//    pwm--;
    pwm-=5;
    mot1.pwm(pwm);
    mot2.pwm(pwm);
  }
  if(counter>768&&counter<1024)
  {
//    pwm++;
    pwm+=5;
    mot1.pwm(pwm);
    mot2.pwm(pwm);
  }
  if(counter>1024)counter=0;
  
//  delay(1000);
  delay(100);  // wait a little bit

  speed1=adc2-OFFSET;
  if(speed1<0) speed1=-speed1;
  speed2=adc3-OFFSET;
  if(speed2<0) speed2=-speed2;

  cur1=adc0-CUROFFSET1;
  if(cur1<0) cur1=-cur1;
  cur2=adc1-CUROFFSET2;
  if(cur2<0) cur2=-cur2;

  Serial.print("AIN: "); Serial.print(adc0);
  Serial.print(" "); Serial.print(adc1);
  Serial.print(" "); Serial.print(adc2);
  Serial.print(" "); Serial.print(adc3);
  Serial.print(" AIN : "); Serial.print(adc);
  Serial.print(" Vin : "); Serial.print(vin);
  Serial.print(" -*- "); Serial.print(speed1);
  Serial.print(" "); Serial.print(speed2);
  Serial.print(" -*- "); Serial.print(cur1);
  Serial.print(" "); Serial.print(cur2);
  Serial.println(" ");
  DOG.string(0,0,UBUNTUMONO_B_16,itoa(counter,buffer,10),ALIGN_CENTER);
  DOG.string(0,0,UBUNTUMONO_B_16,itoa(emergencystop,buffer,10),ALIGN_LEFT);
  DOG.string(0,0,UBUNTUMONO_B_16,itoa(vin*100,buffer,10),ALIGN_RIGHT);
  
}
