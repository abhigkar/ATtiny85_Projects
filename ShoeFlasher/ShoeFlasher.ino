#include <Arduino.h>

//             ATtiny85
//              +-\/-+
//[RESET]  PB5 1|*   |8     [VCC] 
//(G)      PB3 2|    |7 PB2 [SCK] (INT0)
//         PB4 3|    |6 PB1 [MISO](B)
//[GND]        4|    |5 PB0 [MOSI](R)
//              +----+
// Current measurement using a CR2032 button cell
// Red        - 12.50mA
// Green      -  4.00mA
// Blue       -  3.40mA
// sleep WDT  -   4.2uA
// sleep INT0 -   0.1uA

//#define DEBUG_POWERUSE 1
#if defined(DEBUG_POWERUSE)
void powerUse();
#endif

#define BLUE_PIN 1
#define RED_PIN 0
#define GREEN_PIN 3

uint8_t cIdx = 0;

uint8_t colourmap[][3] = {
	{ 1, 0, 0 }, // 0: Red
	{ 0, 1, 0 }, // 1: Green
	{ 0, 0, 1 }, // 2: Blue
	{ 1, 0, 1 }, // 3: Pink
	{ 0, 1, 1 }, // 4: Aqua
	{ 1, 1, 0 }, // 5: Yellow
	{ 1, 1, 1 }  // 6: White
};

#include <avr_sleep.h>

avr_sleep attiny85;

void flash(const uint8_t i = 2, const uint8_t prescalar = 3);

void setup()
{
	// setup the attiny85 with default low power settings (like all pins as inputs)
	attiny85.setup();

	// Set Button pins as OUTPUT
	pinMode(RED_PIN, OUTPUT);
	pinMode(GREEN_PIN, OUTPUT);
	pinMode(BLUE_PIN, OUTPUT);

#if defined(DEBUG_POWERUSE)
	// Flash Blue
	digitalWrite(BLUE_PIN, HIGH);
	attiny85.sleep_wdt(4);
	digitalWrite(BLUE_PIN, LOW);
	attiny85.sleep_wdt(4);
	digitalWrite(BLUE_PIN, HIGH);
	attiny85.sleep_wdt(4);
	digitalWrite(BLUE_PIN, LOW);
	attiny85.sleep_wdt(4);

	// Wait for 10seconds
	attiny85.sleep_wdt(9);
	attiny85.sleep_wdt(7);

	// Flash Blue
	digitalWrite(BLUE_PIN, HIGH);
	attiny85.sleep_wdt(4);
	digitalWrite(BLUE_PIN, LOW);
	attiny85.sleep_wdt(4);
	digitalWrite(BLUE_PIN, HIGH);
	attiny85.sleep_wdt(4);
	digitalWrite(BLUE_PIN, LOW);
	attiny85.sleep_wdt(4);
#endif
}

void loop()
{
#if defined(DEBUG_POWERUSE)
	powerUse();
#else
	// wait for the vibration sensor to trigger an external interrupt
	attiny85.sleep_int0();

	// Flash blue 3x
	flash(2, 3); //125ms on, 125ms off
	flash(2, 3); //125ms on, 125ms off
#endif
}

void flash(const uint8_t i, const uint8_t prescalar)
{
	// flash high
	digitalWrite(RED_PIN, colourmap[i][0]);
	digitalWrite(GREEN_PIN, colourmap[i][1]);
	digitalWrite(BLUE_PIN, colourmap[i][2]);

	attiny85.sleep_wdt(prescalar);

	// flash low
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, LOW);

	attiny85.sleep_wdt(prescalar);
}

#if defined(DEBUG_POWERUSE)
void powerUse()
{
	// RED
	attiny85.sleep_int0(); 
	digitalWrite(RED_PIN, HIGH);
	attiny85.sleep_wdt(7);

	// GREEN
	attiny85.sleep_int0();
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, HIGH);
	attiny85.sleep_wdt(7);

	// BLUE
	attiny85.sleep_int0();
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, HIGH);
	attiny85.sleep_wdt(7);

	// 2 LEDS : AQUA
	attiny85.sleep_int0();
	digitalWrite(GREEN_PIN, HIGH);
	attiny85.sleep_wdt(7);

	// 3 LEDS : WHITE
	attiny85.sleep_int0();
	digitalWrite(RED_PIN, HIGH);
	attiny85.sleep_wdt(7);

	// Turn off the LEDs and sleep on the watchdog timer
	attiny85.sleep_int0();
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, LOW);
	attiny85.sleep_wdt(9); // 8s
	attiny85.sleep_wdt(9); // 16s
	attiny85.sleep_wdt(9); // 24s
	attiny85.sleep_wdt(9); // 32s
	attiny85.sleep_wdt(9); // 40s
	attiny85.sleep_wdt(9); // 48s
	attiny85.sleep_wdt(9); // 56s
	attiny85.sleep_wdt(8); // 60s
}
#endif