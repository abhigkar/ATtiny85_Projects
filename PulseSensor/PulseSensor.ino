/*  Pulse Sensor Amped 1.4    by Joel Murphy and Yury Gitman   http://www.pulsesensor.com
*/

/*
ATtiny85
		       +-\/-+
RST A0 D5 PB5 1|*   |8 VCC
PWM A3 D3 PB3 2|    |7 PB2 D2 A1     SCK  SCL
PWM A2 D4 PB4 3|    |6 PB1 D1    PWM MISO
	      GND 4|    |5 PB0 D0    PWM MOSI SDA
		       +----+
*/

//  Variables
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduino finds a beat.

#include "PulseSensor_Interrupt.h"

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#if DEBUG_LEVEL
#if defined(__AVR_ATtiny85__)
#include <SoftwareSerial.h>
SoftwareSerial uart(3,4);
#else
#define uart Serial
#endif
#endif

#if DEBUG_LEVEL
void serialOutputWhenBeatHappens()
{
	uart.print(F("*** Heart-Beat Happened *** "));  //ASCII Art Madness
	uart.print(F("BPM: "));
	uart.print(BPM);
	uart.println("");
}
#endif

void setup(){
  #if DEBUG_LEVEL
  uart.begin(115200);             // we agree to talk fast!
  uart.println(F("</setup>")); 
  #endif
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
   // IF YOU ARE POWERING The Pulse Sensor AT VOLTAGE LESS THAN THE BOARD VOLTAGE, 
   // UN-COMMENT THE NEXT LINE AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);
}


//  Where the Magic Happens
void loop(){   
  /*if (QS == true) // A Heartbeat Was Found
  {     
        #if DEBUG_LEVEL
		uart.println(F("^_"));
        serialOutputWhenBeatHappens();   // A Beat Happened, Output that to uart.     
        #endif
        QS = false;                      // reset the Quantified Self flag for next time    
  }
  //delay(20);                             //  take a break
  */
#if DEBUG_LEVEL
	uart.print(F("BPM: "));
	uart.println(BPM);
#endif

	delay(1000);                             //  take a break

}
