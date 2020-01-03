
#include <current_control.h>
#include <iostream>

int main(){
  ports_t ports = {4, 14, 15, 18};
  ADS1115_ADC a(0,0);
  //Use the initialization to stop the LEDs
  LEDS leds(ports);
  return 0;
}
