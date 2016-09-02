/* Tiny GPS Speedometer

   David Johnson-Davies - www.technoblogy.com - 10th December 2014
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
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

/*
#ifndef __AVR_ATtiny85__
#define __AVR_ATtiny85__
#endif

#ifdef __AVR_ATtiny85__
#include <TinyPinChange.h>
#include <SoftSerial.h> // For digispark / digistump
#define SoftwareSerial SoftSerial
#else
*/
#include <SoftwareSerial.h>
//#endif

SoftwareSerial uart(2,3); // RX,TX
#include <RingBuffer.h>
RingBuffer<float> speed;
#define _DEBUG 0

// Constants
#define knots2kph 1852 // 0.51444444444;
#define knots2mps 1852/(60*60) // 0.51444444444;

// ParseGPS
// Examples: 
// $GPRMC,194509.000,A,4042.6142,N,07400.4168,W,2.03,221.11,160412,,,A*77
// $GPZDA,035751.000,08,12,2015,,*5E

char fmt[] = "$GPRMC,dddddt.ddm,A,eeae.eeee,l,eeeae.eeee,o,djdk,ddd.dc,dddddy,,,A*??";
char rmc[] = "$GPRMC,dddddt.ddm,A,eeae.eeee,l,eeeae.eeee,o,djdk,ddd.dc,dddddy,,,A*??";
char zda[] = "$GPZDA,dddddt.ddm,dD,dM,dddY,,*??";
char msg[] = "$$$$$$";

long t0,t1 = 0;
float kph = 0.0f;
float kph_avg = 0.0f;
int state = 0;
unsigned int temp;
long ltmp;

// GPS variables
volatile unsigned int Time, Msecs, Knots, Course, Date;
volatile unsigned int Day, Month, Year;
volatile long Lat, Long;

void ParseGPS (char c)
{
  if (c == '$') { state = 0; temp = 0; }
  if (state < 6) {msg[state] = c;}
  
  char mode = fmt[state++];
  
  // If the format is '?' - return
  if (mode == '?') { msg[5] = '?'; return; }

  // Process $GPRMC messages
  if (strcmp(msg,"$GPRMC"))
  {
    char d = c - '0';
    // d=decimal digit
    if (mode == 'd') { temp = temp*10 + d; }
    // t=Time - hhmm
    else if (mode == 't') { Time = temp*10 + d; temp = 0; }
    // m=Millisecs
    else if (mode == 'm') { Msecs = temp*10 + d; temp = 0; ltmp = 0; }
    // l=Latitude - in minutes*10000
    else if (mode == 'l') { if (c == 'N') Lat = ltmp; else Lat = -ltmp; temp = 0; ltmp = 0; }
    // o=Longitude - in minutes*10000
    else if (mode == 'o') { if (c == 'E') Long = ltmp; else Long = -ltmp; temp = 0; ltmp = 0; }
    // j/k=Speed - in knots*100
    else if (mode == 'j') { if (c != '.') { temp = temp*10 + d; state--; } }
    else if (mode == 'k') { Knots = temp*10 + d; temp = 0; }
    // c=Course (Track) - in degrees*100
    else if (mode == 'c') { Course = temp*10 + d; temp = 0; }
    // y=Date - ddmm
    else if (mode == 'y') { Date = temp*10 + d ; }
    else state = 0;
  }
  // Process $GPRMC messages
  else if (strcmp(msg,"$GPZDA"))
  {
    char d = c - '0';
    // d=decimal digit
    if (mode == 'd') { temp = temp*10 + d; }
    // t=Time - hhmm
    else if (mode == 't') { Time = temp*10 + d; temp = 0; }
    // m=Millisecs
    else if (mode == 'm') { Msecs = temp*10 + d; temp = 0; }
    // D=Day
    else if (mode == 'D') { Day = temp*10 + d; temp = 0; }
    // M=Month
    else if (mode == 'M') { Month = temp*10 + d; temp = 0; }
    // Y=Year
    else if (mode == 'Y') { Year = temp*10 + d; temp = 0; }
    else state = 0;
  }
}

void setup (void)
{
  uart.begin(9600);
  t0 = micros();
  t1 = t0;
  uart.println(F("Start"));
}

void loop (void)
{
  if (uart.available())
  {
    ParseGPS(uart.read());

    // update at the end of a parsed string
    if (strcmp(msg,"$GPRM!"))
    {
      uart.println(F("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
      kph = (float)(Knots / 100.0f) * knots2kph;
      speed.set(kph);
      kph_avg = speed.average();
    }
  }
  
  // Output every second
  if (millis() - t0 >= 1000)
  {
    t0 = millis();
    uart.print(F("Speed: "));
    uart.print(Knots);///100);
    uart.print(F("(centi knots), "));
    uart.print(kph * 100.0f);
    uart.print(F("(ckph), "));
    uart.print(kph_avg * 100.0f);
    uart.println(F("(AVG ckph)"));

    uart.print(F("Date: "));
    uart.print(Date);
    uart.print(F("Time: "));
    uart.print(Time);
    uart.print(F("Lat: "));
    uart.print(Lat);
    uart.print(F("Long: "));
    uart.println(Long);
  }
  // Output every 250ms
  if (millis() - t1 >= 250)
  {
    t1 = millis();
    uart.print(F("msg = "));
    uart.print(msg[0]);uart.print(msg[1]);uart.print(msg[2]);uart.print(msg[3]);uart.print(msg[4]);uart.println(msg[5]);
  }
}

