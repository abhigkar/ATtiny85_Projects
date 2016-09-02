#ifndef pwm_h
#define pwm_h

#include <Arduino.h>

/*
-----------+-----------+-----------------
Chip       | #define   | WWVB_OUT
-----------+-----------+-----------------
ATtiny85   |  USE_OC1A | D1 / PB1 (pin 6)
ATtiny85   | *USE_OC1B | D4 / PB4 (pin 3)
-----------+-----------+-----------------
ATmega32u4 | *USE_OC1A | D9
ATmega32u4 |  USE_OC1B | D10
-----------+-----------+-----------------
ATmega328p | *USE_OC1A | D9
ATmega328p |  USE_OC1B | D10
-----------+-----------+-----------------

* Default setup
*/
#if !(defined(USE_OC1A) | defined(USE_OC1B))
#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
#define USE_OC1B
#if !defined(_DEBUG)
#define _DEBUG 0
#endif
#else
#define USE_OC1A
#if !defined(_DEBUG)
#define _DEBUG 2
#endif
#endif
#endif

class pwm
{
   private:
   public:
   uint8_t pulse_width, period;
   pwm() : pulse_width(66), period(133) {};
   void setup()
   {
      // Generate 60kHz carrier
      #if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
        /*
        OC0A | !OC1A : 0 PB0 MOSI PWM 8b
        OC1A |  OC0B : 1 PB1 MISO PWM 8b
        OC1B : 4 PB4
        !OC1B : 3 PB3
        */
        //PLLCSR = 0; // clear all flags, use synchronous clock clock (CK)
        
        TCCR1 = _BV(CS10); 		// prescalar = 1 (8MHz/1 = 8MHz)
        
        OCR1C = period;
        OCR1B = pulse_width;
        /*
        #if defined(USE_OC1A)
        pinMode(PB1, OUTPUT);	// setup OC1A PWM pin as output
  
        TCCR1 |= _BV(PWM1A)  	// Clear timer/counter after compare match to OCR1C
        | _BV(COM1A1); 	// Clear the OC1A output line after match
  
        TIMSK |= _BV(OCIE1A);	// enable compare match interrupt on Timer1
  
        OCR1A = period; 		// Set pulse width to 5% duty cycle
        #elif defined(USE_OC1B)
        */
        pinMode(PB4, OUTPUT);	// setup OC1B PWM pin as output
        
        GTCCR = _BV(PWM1B) 		// Clear timer/counter after compare match to OCR1C
        | _BV(COM1B1);	// Clear the OC1B output line after match
  
        TIMSK = _BV(TOIE1);	// enable interrupt on Timer1 overflow
      #else // This is a catchall for any other chip, but ive only tested it against the chips listed below
        /*
        ATmega32u4
        OC0A |  OC1C : 11 PB7 PWM 8/16b
        OC0B :  3 PD0 SCL PWM 8b | 18 PWM 10b?
        OC1A | !OC4B :  9 PB5 PWM 16b
        OC1B |  OC4B : 10 PB6 PWM 16b
        OC3A |  OCB4 :  5 PC6 PWM HS
        OC4A : 13 PC7 PWM 10b
        !OC4D : 12 PD6 PWM 16b
  
        ATmega328p
        OC0A :  6 PD6
        OC0B :  5 PD5
        OC1A :  9 PB1
        OC1B : 10 PB2 !SS
        OC2A : 11 PB3 MOSI
        OC2B :  3 PD3
        */
        // User Phase & Frequency correct PWM for other chips (leonardo, uno et. al.)
        TCCR1B = _BV(WGM13); // Mode 8: Phase & Frequency correct PWM
  
        #if (F_CPU == 16000000)
        ICR1 = period; // Set PWM to 60kHz (16MHz / (2*133)) = 60150Hz
        // Yes, its 133 and not 132 because the compare is double sided : 0->133->132->1
        #elif (F_CPU == 8000000)
        ICR1 = period/2; // Set PWM to 60.6kHz (8MHz / (2*66)) = 60606Hz
        #endif
  
  
        #if defined(USE_OC1A)
        pinMode(9, OUTPUT);
  
        TCCR1A = _BV(COM1A1); // Clear OC1A on compare match to OCR1A
  
        OCR1A = PWM_LOW;
        #elif defined(USE_OC1B)
        pinMode(10, OUTPUT);
  
        TCCR1A = _BV(COM1B1); // Clear OC1B on compare match to OCR1B
  
        OCR1B = pulse_width;
        #endif

        TIMSK1 = _BV(TOIE1); // enable interrupt on Timer1 overflow
      #endif
      
	  sei(); // enable interrupts
   }

   void start()
   {
      #if defined(USE_OC1A)
      OCR1A = pulse_width;   // Set PWM to 5% duty cycle (signal LOW)
      #elif defined(USE_OC1B)
      OCR1B = pulse_width;   // Set PWM to 5% duty cycle (signal LOW)
      #endif
      resume();
   }
   void set_low()
   {
      #if defined(USE_OC1A)
      OCR1A = 10;//period*0.1;   // Set PWM to 5% duty cycle (signal LOW)
      #elif defined(USE_OC1B)
      OCR1B = 10;//period*0.1;   // Set PWM to 5% duty cycle (signal LOW)
      #endif
   }
   void set_high()
   {
      #if defined(USE_OC1A)
      OCR1A = 100;//period*0.9;   // Set PWM to 50% duty cycle (signal LOW)
      #elif defined(USE_OC1B)
      OCR1B = 100;period*0.9;   // Set PWM to 50% duty cycle (signal LOW)
      #endif
   }
   void set_mid()
   {
      #if defined(USE_OC1A)
      OCR1A = 50;//period*0.5;   // Set PWM to 50% duty cycle (signal LOW)
      #elif defined(USE_OC1B)
      OCR1B = 50;//period*0.5;   // Set PWM to 50% duty cycle (signal LOW)
      #endif
   }
   
   void stop()
   {
      #if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
      TCCR1 &= ~( _BV(CS13) | _BV(CS12) |_BV(CS11) |_BV(CS10) ); // Set clock prescalar to 000 (STOP Timer/Clock)
      #else
      TCCR1B &= ~( _BV(CS12) |_BV(CS11) |_BV(CS10) ); // Set clock prescalar to 000 (STOP Timer/Clock)
      #endif
   }
   void resume()
   {
      #if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
      TCCR1 |= _BV(CS12); // prescalar = 1
      #else
      TCCR1B |= _BV(CS10); // prescalar = 1
      #endif
   }
};

#endif
