#ifndef I2C_H
#define I2C_H

#include <unistd.h>
#include <pigpio.h>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>

#endif

void selectDevice(int fd, int addr, std::string name);
void writeToDevice(int fd, int reg, int val);
void writeToDeviceShort(int fd, int reg, int val);
short readShort(int fd, int reg);
short readShortInv(int fd, int reg);
long long readLong(int fd, int reg, int bytes);
short readByte(int fd, int reg);
