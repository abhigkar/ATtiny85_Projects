#include <Arduino.h>
#include <SoftwareSerial.h>

int pinState = LOW;

unsigned long tInterval = 1000;
unsigned long tStop = tInterval;

#define ledPin 3

#define TX_PIN 4
#define RX_PIN 0

unsigned int count = 0;
SoftwareSerial sSerial;

void setup()
{
  sSerial = SoftwareSerial(RX_PIN,TX_PIN);
  // Setup pin connections  
  pinMode(ledPin, OUTPUT);

  // Setup serial connections
  sSerial.begin(9600);
}

void heartbeat()
{
  if (millis() > tStop)
  {
    digitalWrite(ledPin, pinState);
    
    pinState = !pinState;
    tStop = millis() + tInterval;
    
    sSerial.print("/\\_");
    
    if (++count>4)
    {
      sSerial.println("");
      count = 0;
    }
  }  
}

void loop()
{
  heartbeat();
  
  delay(1);
}

