#ifndef AMP_H
#define AMP_H

#include <chrono>
#include <ctime>
#include <current_control.h>
#include <iostream>
#include <pigpio.h>

template <typename T> int sign(T val) { return (T(0) < val) - (val < T(0)); }

/**
 * @brief Calculates the target current for the LEDs.
 *
 * @param light The index of the LED considered
 * @param time The time remaining until full brightness
 *
 * @return
 */
float get_target_current(int light, float time);

/**
 * @brief Interface for a high resolution clock from chrono
 *
 * @return Returns a std::chrono::time_point representing the current value of the clock
 */
std::chrono::high_resolution_clock::time_point getTime();

/**
 * @brief
 *
 * @return
 */
int read_time();

#endif
