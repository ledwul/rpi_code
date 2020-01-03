#include <current_control.h>
#include <iostream>

int main(){
  ports_t ports = {4, 14, 15, 18};
  ADS1115_ADC a(0,0);
  LEDS leds(ports);
  while(true){
    leds.update();
  }
  return 0;
}
