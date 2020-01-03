#ifndef CURRENT_CONTROL_H
#define CURRENT_CONTROL_H

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <i2c.h>
#include <iostream>
#include <pigpio.h>
#include <vector>

using ports_t = std::vector<int>;
using voltages_t = std::vector<float>;

class ADS1115_ADC {
private:
  const int i2c_addr_;
  const int config_adr = 0x1;
  const int result_adr = 0x0;
  const float shunt_ohms_;
  int fd_;
  short prev_output;

public:
  voltages_t offsets_;
  /**
   * @brief Open selected i2c device.
   *
   * @param i2c_addr I2C adress of the selected device
   * @param config ?
   * @param result ?
   * @param shunt_ohms Resistance of the used shunt resistor
   */
  ADS1115_ADC(int i2c_addr, float shunt_ohms);
  /**
   * @brief Get the voltage measured on the selected channel
   *
   * @param channel Integer between 0 and 3 denoting one of the 4 channels of
   * the ADS1115
   *
   * @return Voltage of the selected channel in [volt]
   */
  float getVoltage(int channel);
  /**
   * @brief Get the current on the selected channel
   *
   * @param channel Integer between 0 and 3 denoting of the 4 channels of the
   * ADS1115
   *
   * @return Current of the selected channel in [ampere]
   */
  float getCurrent(int channel);

  /**
   * @brief Registers the voltage offsets on all channels by setting pwm output
   * to zero and measuring the voltage over the shunt resistors.
   *
   * @param ports Vector of ints containing the pwm ports
   */
  void setOffsets(ports_t ports);
};

class LEDS {
private:
  ports_t PORTS;
  std::vector<int> led_pwm_values = std::vector<int>(4, 0);
  std::vector<float> led_current_measurements = std::vector<float>(4, 0);
  const int I2C_ADDR = 0x48;
  const float SHUNT_OHMS = 0.5;
  const int STEPS = 30000;
  const float P_CONTROL = 0.01;
  const float D_CONTROL = 30;
  const float MAXIMUM_BRIGHTNESS = 0.3;
  ADS1115_ADC *adc_handler = 0;
  int update_counter = 0;
  int total_time_until_wakeup;
  std::chrono::high_resolution_clock::time_point time_of_initialisation;
  /**
   * @brief This function calculates the total_time_until_wakeup and registers
   * the time of its execution. Shall only be called when the alarm clock should
   * be reset to a new wake up time.
   */
  void update_time_until_wakeup();
  /**
   * @brief This function calculates the desired brightness of the LEDs,
   * depending on the time remaining until wakeup, and then returns the
   * necessary current for the chosen LED.
   *
   * @param light index of the LED.
   * @param time Time remaining until wake up.
   *
   * @return Desired current in Ampere.
   */
  float get_target_current(int light, float time);
  std::chrono::high_resolution_clock::time_point get_time();
  /**
   * @brief Function that accesses the file saved by the website service, once
   * the wake up time has been entered.
   *
   * @return Wake up time in UTC.
   */
  int read_time_sec_UTC();

public:
  /**
   * @brief Constructor for LEDS, does the following things:
   * 1. Initializes the gpio pins
   * 2. Updates the wake up time
   * 3. Initializes an adc_handler
   * 4. Initialize the GPIO pin range and sets the current output to zero
   *
   * @param ports Vector containing the indices of the rasperry pi ports
   * delivering the PWM signal for the LEDs
   */
  LEDS(ports_t ports);
  /**
   * @brief This function executes all necessary steps for keeping watch on
   * changes of the desired wake up time, calculation of the remaining time
   * until wake up and the setting of the desired LED brightness and needs to be
   * executed as frequently as possible.
   */
  void update();
};

#endif
