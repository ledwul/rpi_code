#include "current_control.h"

const int alert_pin = 26;
ADS1115_ADC::ADS1115_ADC(int i2c_addr, float shunt_ohms)
    : i2c_addr_(i2c_addr), shunt_ohms_(shunt_ohms) {

  try {
    fd_ = open("/dev/i2c-1", O_RDWR);
    if (fd_ < 0)
      throw 2;
  } catch (int e) {
    fprintf(stderr, "Failed to open i2c bus\n");
  }
  gpioSetMode(alert_pin, PI_INPUT);

  selectDevice(fd_, i2c_addr_, "ADS1115");
}

float ADS1115_ADC::getVoltage(int channel) {
  long sum = 0;
  const int n_samples = 1;
  short output_buffer = prev_output;
  for (int i = 0; i < n_samples; i++) {
    int cnt = 0;
    while ((readShort(fd_, config_adr) & 0b1000000000000000) == 0)
      cnt++; // wait for previous conversion to finish
    output_buffer = readShort(fd_, result_adr);

    //                                    1222333455567899
    writeToDeviceShort(fd_, config_adr, 0b1100100111100011 | (channel << 12));

    return output_buffer / (32768.0 * 0.512);
    sum += output_buffer;
  }
  prev_output = output_buffer;
  return sum / 32768.0 / n_samples * 0.512;
}

float ADS1115_ADC::getCurrent(int channel) {
  float voltage = getVoltage(channel);
  return (voltage - offsets_.at(channel)) / shunt_ohms_;
}

void ADS1115_ADC::setOffsets(ports_t ports) {
  // Check of offset is already populated, populate if not
  if (offsets_.size() == 0) {
    offsets_.resize(ports.size());
  }
  // else check if sizes agree
  else {
    assert(offsets_.size() == ports.size());
  }

  for (int i = 0; i < 4; i++) {
    // Set pwm output to zero
    gpioWrite(ports.at(i), 0);
    usleep(100000);
    // Get the voltage on zero pwm
    float sum = 0;
    offsets_[i] = 0;
    for (int j = 0; j < 100; j++)
      sum += getVoltage(i);
    offsets_[i] = sum / 100.0;
    std::cerr << "Offset for channel " << i << " is set to " << offsets_[i]
              << "V." << std::endl;
  }
}

LEDS::LEDS(ports_t ports) : PORTS(ports) {
  // Initialize GPIO
  try {
    if (gpioInitialise() < 0)
      throw 1;
  } catch (int e) {
    std::cout << "GPIO pins could not be initialized!" << std::endl;
  }

  this->update_time_until_wakeup();

  // Initialize adc_handler
  this->adc_handler = new ADS1115_ADC(I2C_ADDR, SHUNT_OHMS);
  adc_handler->setOffsets(this->PORTS);

  // Initialize GPIO pin range and set current output to zero
  for (int i = 0; i < 4; i++) {
    gpioSetPWMrange(this->PORTS.at(i), this->STEPS);
    gpioPWM(this->PORTS.at(i), 0);
  }
}

void LEDS::update_time_until_wakeup() {
  this->total_time_until_wakeup = read_time_sec_UTC() - time(0);
  this->time_of_initialisation = get_time();
}

void LEDS::update() {
  this->update_counter++;

  if (this->update_counter % 10000 == 0) {
    this->update_time_until_wakeup();
  }

  float remaining_time_until_wakeup =
      ((std::chrono::duration<float>)(get_time() -
                                      this->time_of_initialisation))
          .count() -
      this->total_time_until_wakeup;

  for (int i = 0; i < 4; i++) {
    float target_current = get_target_current(i, remaining_time_until_wakeup);

    // Record the actual current for this light
    led_current_measurements.at(i) = adc_handler->getCurrent(i);

    if (target_current < 0.01) {
      // Set pwm output to zero if target current is too small
      led_pwm_values.at(i) = 0;
    } else {
      // P control of current
      int delta_current = lround(
          this->P_CONTROL * (target_current - led_current_measurements.at(i)) *
          this->STEPS);

      // Limit the maximum current change
      if (delta_current > this->D_CONTROL) {
        delta_current = this->D_CONTROL;
      } else if (delta_current < -this->D_CONTROL) {
        delta_current = -this->D_CONTROL;
      }
      led_pwm_values.at(i) += delta_current;
    }

    // Set the chosen current limited by [0, steps]
    gpioPWM(this->PORTS.at(i),
            std::max(0, std::min(this->STEPS, led_pwm_values.at(i))));

    // Output status info all 1000 iterations
    if (this->update_counter % 1000 == 0) {
      std::cout << "Light " << i << ": " << this->led_current_measurements.at(i)
                << "A (" << this->led_pwm_values.at(i) << "), "
                << adc_handler->getVoltage(i) - adc_handler->offsets_[i]
                << "V, Target: "
                << get_target_current(i, remaining_time_until_wakeup) << "A."
                << std::endl;
    }
  }
}

float LEDS::get_target_current(int light, float time) {

  if (time > 0)
    // It's already time for full brightness
    return this->MAXIMUM_BRIGHTNESS;

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

  return this->MAXIMUM_BRIGHTNESS * (ramp_time + time) / ramp_time;
}

std::chrono::high_resolution_clock::time_point LEDS::get_time() {
  return std::chrono::high_resolution_clock::now();
}

int LEDS::read_time_sec_UTC() {
  std::ifstream myfile("/home/pi/wakeuptime.txt");
  int target_time;
  if (myfile.is_open()) {
    myfile >> target_time;
  } else {
    std::cerr << "wakeuptime.txt not found!" << std::endl;
    target_time = time(0) + 25;
  }
  return target_time;
}
