#include "PWM.h"

pwm t_pwm;

ISR(TIMER1_OVF_vect)
{
  // do nothing
}

uint32_t t0;
uint8_t state = 0;
void setup()
{
  t_pwm.setup();
  t_pwm.start();
  
  t0 = millis();
}

void loop()
{
  if(millis() - t0 >= 1000)
  {
    switch(state++)
    {
      case 0:
      t_pwm.set_low(); break;
      case 1:
      t_pwm.set_mid(); break;
      default:
      t_pwm.set_high(); 
      state = 0;
      break;
    }
  }
}
