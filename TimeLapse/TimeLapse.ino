#include <Arduino.h>

//                   ATtiny85
//                    +-\/-+
//           RST PB5 1|*   |8          [VCC]
// [POWER_PIN]   PB3 2|    |7 PB2 SCK  
// [SHUTTER_PIN] PB4 3|    |6 PB1 MISO [SLEEP_PIN]
// [GND]             4|    |5 PB0 MOSI 
//                    +----+
//
// +------------+-----+-----+------+-------+
// |            | WDT |  1m |  1hr |   4hr |
// +------------+-----+-----+------+-------+
// | secs       |   8 |  60 | 3600 | 14400 |
// +------------+-----+-----+------+-------+
// | multiplier |   1 | 7.5 |  450 |  1800 |
// +------------+-----+-----+------+-------+
//

/*
Power save stuff based off:
Low Power Testing
Spark Fun Electronics 2011
Nathan Seidle
https://www.sparkfun.com/tutorials/309

Low power blinky test for 8MHz boards
Note: This will work with other clock frequencies - but the delay routine will be wrong
*/

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down peripherals such as the ADC/TWI and Timers
#include <util/atomic.h>

#define POWER_PIN 3
#define SHUTTER_PIN 4
#define SLEEP_PIN 1
#define INT0_PIN 2
#define MAX_PINS 6

#ifndef WDTCSR
#define WDTCSR WDTCR // i.e. for the ATtiny85
#endif

volatile uint8_t *INT0_PCMSK;
uint8_t INT0_PCMSK_bit;

// Function declaration
void enable_ExternalInterrupt();
void disable_ExternalInterrupt();
void disable_peripherals();
void sleep(uint32_t NUM_WDT_INTERVALS = 1);
void power_down();

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=125ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
void enable_watchdogInterrupt(uint8_t timerPrescaler = 9); // Default to 8s
void soft_delay_ms(uint16_t x);
void soft_delay_us(uint16_t x);

// ISR : Watchdog Interrupt
ISR(WDT_vect) {}
// ISR : External Interrupt
ISR(INT0_vect) {}

#define ACTIVE HIGH

void setup()
{
	//To reduce power, setup all pins as inputs with no pullups
	// (saves about 1.3uA when used with watchdog timer in this example)
	for (uint8_t x = 0; x < MAX_PINS; ++x)
	{
		pinMode(x, INPUT);
		digitalWrite(x, (x == 2)); // set PB2 / INT0 high
	}

	// If SLEEP_PIN is jumpered HIGH, put the ATtiny to sleep (permanently)
	if (digitalRead(SLEEP_PIN) == HIGH)
	{
		disable_ExternalInterrupt();
		power_down();// Go to sleep
	}

	// Set Button pins as outputs
	pinMode(POWER_PIN, OUTPUT);
	pinMode(SHUTTER_PIN, OUTPUT);

	//Power down various bits of hardware to lower power usage
	disable_peripherals();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	//setup wakeup on external trigger
	enable_ExternalInterrupt();
	power_down();// Go to sleep
	disable_ExternalInterrupt();

	//Setup wakeup on watchdog timer
	//Set watchdog to go off after 1sec
	enable_watchdogInterrupt();
}

void loop()
{
	// Power ON
	digitalWrite(POWER_PIN, ACTIVE);
	soft_delay_ms(500);
	digitalWrite(POWER_PIN, !ACTIVE);
	// Switch to photo mode
	soft_delay_ms(1000);
	digitalWrite(POWER_PIN, ACTIVE);
	soft_delay_ms(500);
	digitalWrite(POWER_PIN, !ACTIVE);
	// Take the photos (should take 4 - time lapse of 0.25s has been set on the camera)
	digitalWrite(SHUTTER_PIN, ACTIVE);
	soft_delay_ms(1000);
	digitalWrite(SHUTTER_PIN, !ACTIVE);
	// Power OFF
	digitalWrite(POWER_PIN, ACTIVE);
	soft_delay_ms(4000);
	digitalWrite(POWER_PIN, !ACTIVE);

	sleep(1800); // Go to sleep
	// wake up here
}

void disable_peripherals()
{
	ADCSRA &= ~(1 << ADEN); //Disable ADC
	ACSR = (1 << ACD); //Disable the analog comparator

	DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-3 pins and disable digital input buffer on AIN0,1
	/*
	power_timer0_disable(); //Needed for delay_ms
	power_timer1_disable();
	power_adc_disable();
	power_usi_disable();
	*/

	// disable all peripherals
	power_all_disable();
}

void sleep(uint32_t NUM_WDT_INTERVALS)
{
	for (uint32_t i = 0; i < NUM_WDT_INTERVALS; ++i)
	{
		power_down();
	}
}

void power_down()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		sleep_enable();
		sleep_bod_disable(); // Doesn't save any power. I assume it uses the fuze settings, which have BOD disabled
	}
	sleep_cpu();
	sleep_disable();
}

//Sets the watchdog timer to wake us up, but not reset
//0=16ms, 1=32ms, 2=64ms, 3=125ms, 4=250ms, 5=500ms
//6=1sec, 7=2sec, 8=4sec, 9=8sec
void enable_watchdogInterrupt(uint8_t timerPrescaler)
{
	// WDTCSR = [  WDIF |  WDIE |  WDP3 |  WDCE |  WDE  |  WDP2 |  WDP1 |  WDP0 ]
	uint8_t WDTCSR_ = (timerPrescaler & 0x07);
	WDTCSR_ |= (timerPrescaler > 7) ? _BV(WDP3) : 0x00; // Set WDP3 if prescalar > 7 (ie. 4.0s, 8.0s)
	WDTCSR_ |= _BV(WDIE); // Enable watchdog interrupt

	//This order of commands is important and cannot be combined (beyond what they are below)
	MCUSR &= ~_BV(WDRF); // Clear the watch dog reset

	// timed sequence
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		WDTCSR |= _BV(WDCE) | _BV(WDE); //Set WD_change enable, set WD enable
		WDTCSR = WDTCSR_; //Set new watchdog timeout value & enable
	}
}

void enable_ExternalInterrupt()
{
	// MCUCR = [  BODS |  PUD  |  SE   |  SM1  |  SM0  | BODSE | ISC01 | ISC00 ]
	//MCUCR |= _BV(ISC00); // ISC0[1,0] = [1,0] = rising edge detection
	// ISC0[1,0] = [0,1] = pin change detection
	MCUCR &= ~(_BV(ISC01) | _BV(ISC00)); // 0 : low level generates an interrupt
	//MCUCR &= ~_BV(ISC01); MCUCR |= _BV(ISC00); // 1 : logical change
	//MCUCR |= _BV(ISC01); MCUCR &= ~_BV(ISC00); // 2 : falling edge
	//MCUCR |= _BV(ISC01) | _BV(ISC00); // 3 : rising edge
	PCMSK = 0x00;

	// GIMSK = [   -   |  INT0 |  PCIE |   -   |   -   |   -   |   -   |   -   ]
	GIMSK |= _BV(INT0); // Enable External Interrupt (INT0), disable Pin Change Interrupt

	// Enable the pin interrupt for the INT0 pin (Needed?)
	INT0_PCMSK = digitalPinToPCMSK(INT0_PIN);
	INT0_PCMSK_bit = _BV(digitalPinToPCMSKbit(INT0_PIN));
	*INT0_PCMSK |= INT0_PCMSK_bit;
}

void disable_ExternalInterrupt()
{
	PCMSK = 0x00;
	GIMSK = 0x00;
}

//This is a not-so-accurate delay routine
//Calling soft_delay_ms(100) will delay for about 100ms with a 8MHz clock
void soft_delay_ms(int x)
{
	for (; x > 0; x--)
	{
		soft_delay_us(1000);
	}
}

//This is a not-so-accurate delay routine
//Calling soft_delay_us(100) will delay for about 100us
//Assumes 8MHz clock
void soft_delay_us(int x)
{
	for (; x > 0; x--)
	{
		__asm__("nop\n\t");
		__asm__("nop\n\t");
		__asm__("nop\n\t");
		__asm__("nop\n\t");
		__asm__("nop\n\t");
		__asm__("nop\n\t");
		__asm__("nop\n\t");
	}
}

