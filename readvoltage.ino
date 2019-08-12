#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "LowPower.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define TOP_CHARGE 14600 // top battery voltage
#define BOTTOM_CHARGE 10200 // bottom battery voltage

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/*
 * Using Smoothing example from Arduino public domain 
 *  * created 22 Apr 2007
 * by David A. Mellis  <dam@mellis.org>
 * modified 9 Apr 2012
 * by Tom Igoe
 * http://www.arduino.cc/en/Tutorial/Smoothing
 * 
 * The rest of the code is pieced together from my experimentation and 
 * various examples/tutorials out there on the World Wide Interwebs (WWI)
 * - Rex Vokey (KE6MT) 12 Aug 2019
 */
 
const int numReadings = 20;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
float voltMappedAverage = 0;        // mapped average for voltage
int dispMappedAverage = 0;      // for bar display
float voltage = 0;            // floating point voltage

int top90 = 100;
int middle = 100;
int bottom = 0;
bool battLow = false;


int inputPin = A0;

void setup() {
  // initialize serial communication with computer:
  Serial.begin(19200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(inputPin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // turn into mapped voltage, based on our voltage divider figures
  voltMappedAverage = float(map(average,0,1023,0,15670));
  // bring integer down to floating point voltage
  voltage = voltMappedAverage/1000;
  
  // really crude capacity meter - currently based on 4S LiFePO4
  if (voltMappedAverage > 13000) {
    top90 = map((TOP_CHARGE - voltMappedAverage),0,(TOP_CHARGE - 13000),10,0);
    dispMappedAverage = map(90 + top90,1,100,1,126);
    battLow = false;
  } else if (voltMappedAverage > 12400) {
    middle = map((13000 - voltMappedAverage),0,(13000 - 12400),60,0);
    dispMappedAverage = map(20 + middle,1,100,1,126);
    battLow = false;
  } else {
    bottom = map((12400 - voltMappedAverage),0,(12400 - BOTTOM_CHARGE),20,0);
    dispMappedAverage = map(bottom,1,100,1,126);
    battLow = true;
  }

  // Display all the infos
  display.clearDisplay();
  display.setCursor(10, 0);
  display.setTextSize(2);
  // display warning text if it's time to disconnect (could do other things here, like perform a shutdown)
  if (battLow) {  
    display.println("warning!");
  } else {
    display.println("voltage:");
  }
  display.setTextSize(4);
  display.setCursor(16,22);
  // display voltage with one decimal place
  display.print(voltage,1);
  // capacity bar
  display.drawRect(0,56,128,8, WHITE);
  display.fillRect(0,57,dispMappedAverage,6, WHITE);
  display.display(); 
  LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF); // 250ms = ~4 reads per second, with the µcontroller powered down in between reads
  // if we want to truly go low-power, though, would need to disable LEDs on the Arduino, and only have the oled display on when needed
  // (maybe use a toggle for that?)
  
}
