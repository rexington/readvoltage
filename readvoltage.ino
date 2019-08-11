#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 20;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
float voltMappedAverage = 0;        // mapped average for voltage
int dispMappedAverage = 0;      // for bar display
float voltage = 0;            // floating point voltage

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
  voltMappedAverage = float(map(average,0,1023,0,15670));
  dispMappedAverage = map(average,0,1023,1,126);
  voltage = voltMappedAverage/1000;
  // send it to the computer as ASCII digits
  Serial.println(voltage);
    // Display static text
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("voltage");
  display.setTextSize(4);
  display.setCursor(0,22);
  display.print(voltage);
  display.drawRect(0,56,128,8, WHITE);
  display.fillRect(0,57,dispMappedAverage,6, WHITE);
  display.display(); 
  delay(50);        // delay in between reads for stability
  
}
