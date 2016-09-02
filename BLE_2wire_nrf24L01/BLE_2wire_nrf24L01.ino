/*
 * Emulates a nRF8001 temperature beacon; 
 * reads temperature from a DHT11 and sends it via BTLE.
 * Compatible with Nordic Semiconductor apps such as
 * nRF Master Control Panel or nRF Temp 2.0.
 */
/*
ATtiny85
                               +-\/-+
                RST A0 D5 PB5 1|*   |8 VCC
(RCLK)          PWM A3 D3 PB3 2|    |7 PB2 D2 A1     SCK  SCL (DIO/QH)
(SCLK)          PWM A2 D4 PB4 3|    |6 PB1 D1    PWM MISO     (-> To UART Rx)
                          GND 4|    |5 PB0 D0    PWM MOSI SDA (-> To GPS Tx)
                               +----+
*/
#include <SPI.h>
#include <RF24.h>
#include <BTLE.h>

#include <SoftSerial.h>

SoftSerial ss(1,2);

struct hrm_service_data{
  int16_t   service_uuid;
  int16_t   service_data;
};

RF24 radio(3);
BTLE btle(&radio);
int16_t HRM_temp = NRF_HEARTRATE_SERVICE_UUID;
hrm_service_data HRM_temp2;

void setup() {
  Serial.begin(9600);
  while (!Serial) { }
  Serial.println("BTLE heartrate sender");
  Serial.end();

  HRM_temp2.service_uuid = NRF_HEARTRATE_SERVICE_UUID;
  HRM_temp2.service_data = NRF_HEARTRATE_SERVICE_UUID;
  // 8 chars max
  btle.begin("HRM (Ver");
}

void loop() {
  
  if(!btle.advertise2(0x02, &HRM_temp2, sizeof(HRM_temp2)))
  {
    Serial.println("BTLE advertisement failure");
  }
  btle.hopChannel();
  
  delay(1000);

}

