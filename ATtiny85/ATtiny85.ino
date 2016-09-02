#include <Arduino.h>

/*
ATtiny85
               +-\/-+
RST A0 D5 PB5 1|*   |8 VCC
PWM A3 D3 PB3 2|    |7 PB2 D2 A1     SCK  SCL
PWM A2 D4 PB4 3|    |6 PB1 D1    PWM MISO
          GND 4|    |5 PB0 D0    PWM MOSI SDA
               +----+
*/

#if defined (__AVR_ATtiny85__)
//#include <tinySPI.h>
#else
#include <SPI.h>
#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 2
#endif

#if DEBUG_LEVEL
#ifndef uart
#if defined (__AVR_ATtiny85__)
#include <SoftwareSerial.h>
SoftwareSerial ss(5, 5);
#define uart ss
#else
#define uart Serial
#endif
#endif
#endif

#include <nRF8001.h>
//#include <SoftwareSerial.h>
/*
#include <stdlib.h> // for malloc and free 
void* operator new(size_t size){ return malloc(size); }
void operator delete(void* ptr) { free(ptr); }
*/

// change nRF8001 reset pin to -1 if it's not connected
// Redbear BLE Shield users: to my knowledge reset pin is not connected so use -1!
// NOTE: if you choose -1, you'll need to manually reset your device after powerup!!
#define RESET_PIN -1
#define REQN_PIN 3
#define RDYN_PIN 4
//SoftwareSerial ss()
nRF8001 *nrf;

float temperatureC;
uint8_t pipeStatusReceived, dataSent;
unsigned long lastSent;

#ifndef debugln
#if DEBUG_LEVEL
#define debug uart.print
#define debugln uart.println
#else
#define debug
#define debugln
#endif
#endif

// This function is called when nRF8001 responds with the temperature
void temperatureHandler(float tempC)
{
  debugln(F("received temperature"));
  temperatureC = tempC;
}

// Generic event handler, here it's just for debugging all received events
void eventHandler(nRFEvent *event)
{
  debugln(F("event handler"));
  nrf->debugEvent(event);
}

void setup() {
  #if DEBUG_LEVEL
    uart.begin(115200);
    while (!uart);
  #endif
  temperatureC = 0.0;
  pipeStatusReceived = 0;
  lastSent = 0;

  debugln(F("Hello"));

  // nRF8001 class initialized with pin numbers
  nrf = new nRF8001(RESET_PIN, REQN_PIN, RDYN_PIN);

  // Register event handles
  nrf->setEventHandler(&eventHandler);
  nrf->setTemperatureHandler(&temperatureHandler);
  if ((nrf->setup(NB_SETUP_MESSAGES)) == cmdSuccess) {
    debugln(F("SUCCESS"));
  }
  else {
    debugln(F("FAIL"));
    while (1);
  }

  // These functions merely request device address and temperature,
  // actual responses are asynchronous. They'll return error codes
  // if somehow the request itself failed, for example because
  // the device is not ready for these commands.
  nrf->getDeviceAddress();
  nrf->poll();
  nrf->getTemperature();
  nrf->poll();

  if (temperatureC > 0.0) {
    debug(F("Temperature: "));
    debugln(temperatureC, 2);
  }

  nrf->connect(0, 32);
}

void loop() {
  debugln(F("polling"));

  // Polling will block - times out after 2 seconds
  nrf->poll(2000);

  // If heart rate pipe is open
  if (nrf->isPipeOpen(PIPE_HEART_RATE_HEART_RATE_MEASUREMENT_TX) && (millis() - lastSent) > 1000 && temperatureC > 0.0 && nrf->creditsAvailable()) {
    debugln(F("ready to send data"));
    uint8_t temp[2];
    temp[0] = 0;
    temp[1] = round(temperatureC);

    nrf->sendData(PIPE_HEART_RATE_HEART_RATE_MEASUREMENT_TX, 2, (uint8_t *)&temp);
    lastSent = millis();
    uint8_t bat = 78;

    // If battery pipe is open
    if (nrf->isPipeOpen(PIPE_BATTERY_BATTERY_LEVEL_TX) && nrf->creditsAvailable()) {
      nrf->sendData(PIPE_BATTERY_BATTERY_LEVEL_TX, 1, &bat);
    }

    // get new temperature
    nrf->getTemperature();
  }
  else if (nrf->getConnectionStatus() == Disconnected) {
    debugln(F("Reconnecting"));
    dataSent = 0;
    nrf->connect(0, 32);
  }
}

