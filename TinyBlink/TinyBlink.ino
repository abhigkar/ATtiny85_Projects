// Pin 13 has an LED connected on most Arduino boards.
unsigned char led1 = 5;
unsigned char led2 = 6;

unsigned char led1State = LOW;
unsigned char led2State = LOW;
long led1PrevMillis = 0;
long led2PrevMillis = 0;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin/s as an output.
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop()
{
  blink(led1,1000,led1State,led1PrevMillis);
  delay(1);
  blink(led2,500,led2State,led2PrevMillis);
  delay(1);
}

void blink(const unsigned char ledPin, const long interval, unsigned char &ledState, long &previousMillis)
{
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval)
  {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}
