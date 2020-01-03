#include "amp_new.h"

using namespace std;

const int I2C_ADDR = 0x48;
const float SHUNT_OHMS = 0.50;
short val[4];

int main() {
  // Initialize GPIO
  try {
    if (gpioInitialise() < 0)
      throw 1;
  } catch (int e) {
    cout << "GPIO pins could not be initialized" << endl;
  }

  // Log current time
  auto init = getTime();
  // Initialize Time until wakeup
  int time_until_wakeup = read_time() - time(0);

  // Initialize adc_handler
  ADS1115_ADC adc_handler(I2C_ADDR, SHUNT_OHMS);
  ports_t ports = {4, 14, 15, 18};
  adc_handler.setOffsets(ports);

  vector<float> curs(4);

  // Further initialization of gpio pwm pins
  unsigned int index = 0;
  const short steps = 30000;

  for (int i = 0; i < 4; i++) {
    gpioSetPWMrange(ports.at(i), steps);
    gpioPWM(ports[i], 0);
  }

  while (1) {
    index++;
    // Update wakeup-time
    if (index % 10000 == 0) {
      time_until_wakeup = read_time() - time(0);
    }

    // Calculate how much time remains until wakeup
    float time = ((chrono::duration<float>)(getTime() - init)).count() -
                 time_until_wakeup;

    for (int light = 0; light < 4; light++) {
      float target = get_target_current(light, time);

      // Record the actual current for this light
      curs[(light - 1 + curs.size()) % curs.size()] =
          adc_handler.getCurrent(light);
      float cur = curs[light];

      if (target < 0.01) {
        // Set to zero if target current is too small
        val[light] = 0;
      } else {
        // P control of current
        const float P = 0.01;
        int delta = lround(P * (target - cur) * steps);

        // Define maximum deviation and limit delta
        const int maxdelta = steps / 1000;

        if (delta > maxdelta)
          delta = maxdelta;
        if (delta < -maxdelta)
          delta = -maxdelta;

        val[light] += delta;
      }
      // Set the chosen current, limited by [0,steps]
      gpioPWM(ports[light], max((short)0, min(steps, val[light])));

      // Output status info all 1000 iterations
      if (index % 1000 == 0) {
        cout << "Light " << light << ": " << cur << "A (" << val[light] << "), "
             << adc_handler.getVoltage(light) - adc_handler.offsets_[light]
             << "V, Target: " << get_target_current(light, time) << "A."
             << endl;
      }
    }
  }
  return 0;
}

float get_target_current(int light, float time) {

  const float target_brightness = 0.3;
  if (time > 0)
    // It's already time for full brightness
    return target_brightness;

  float ramp_time;
  // Use different Ramp Times for different LEDs such that we start with the
  // warmer colors
  if (light % 2 == 0) {
    ramp_time = 10;
  } else {
    ramp_time = 20;
  }

  if (time < -ramp_time)
    // It's not time for turning on the LEDs
    return 0;

  return target_brightness * (ramp_time + time) / ramp_time;
}

std::chrono::high_resolution_clock::time_point getTime() {
  return chrono::high_resolution_clock::now();
}

int read_time() {
  ifstream myfile("/home/pi/wakeuptime.txt");
  int target_time;
  if (myfile.is_open()) {
    myfile >> target_time;
  } else {
    cerr << "wakeuptime.txt not found!" << endl;
    target_time = time(0) + 25;
  }
  return target_time;
}
