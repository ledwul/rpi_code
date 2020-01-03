#include "amp_new.h"

using namespace std;

const int I2C_ADDR = 0x48;
const float SHUNT_OHMS = 0.50;

int main() {
  try {
    if (gpioInitialise() < 0)
      throw 1;
  } catch (int e) {
    cout << "GPIO pins could not be initialized" << endl;
  }

  ADS1115_ADC adc_handler(I2C_ADDR, SHUNT_OHMS);
  ports_t ports = {4, 14, 15, 18};
  adc_handler.setOffsets(ports);

  short steps = 30000;
  for (int i = 0; i < 4; i++) {
    gpioSetPWMrange(ports.at(i), steps);
    gpioPWM(ports.at(i), 0);
  }
}

