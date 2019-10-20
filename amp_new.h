#ifndef AMP_H
#define AMP_H

#include <iostream>
#include <chrono>
#include <pigpio.h>
#include <ctime>

template <typename T> int sign(T val) {
  return (T(0) < val) - (val < T(0));
}

#endif

