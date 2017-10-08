#include <Arduino.h>
#include "Radio.h"
#include "RadioHandshake.h"

Radio radio;
RadioHandshake radioHandshake(&radio);

int readAnalogSensorPin(byte pin)
{
  byte measurements = 5;
  float totalMeasurement = 0;
  for (byte i = 0; i < measurements; i++)
  {
    totalMeasurement += analogRead(pin);
  }
  return totalMeasurement / measurements;
}

void setup()
{
  Serial.begin(115200);

  delay(50);
  radio.setup();


  delay(50);
  pinMode(A0, INPUT_PULLUP); // y
}

void loop()
{
  radioHandshake.handleHandshake();
}
