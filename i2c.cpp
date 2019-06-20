#include "i2c.h"

using namespace std;

void selectDevice(int fd, int addr, char * name) {
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, "%s not present\n", name);
    }
}

void writeToDevice(int fd, int reg, int val) {
    char buf[2];
    buf[0] = reg;
    buf[1] = val;

    if (write(fd, buf, 2) != 2) {
        fprintf(stderr, "Can't write to device\n");
    }
}
void writeToDeviceShort(int fd, int reg, int val) {
    char buf[3];
    buf[0] = reg;
    int val1 = val >> 8;
    int val2 = val & 0xFF;
    
    buf[1] = val1;
    buf[2] = val2;

    if (write(fd, buf, 3) != 3) {
        fprintf(stderr, "Can't write to device\n");
    }
}

int onerr=0;
short readShort(int fd, int reg){

	unsigned char buf[16];
	buf[0]=reg;
	if (write(fd,buf,1) != 1){
		cout << "MPU6050: can't tell device to read from buffer" << endl;
		onerr = 10;
	}
	if (read(fd,buf,2) != 2){
		cout << "MPU6050: can't read from device" << endl;
		onerr = 10;
	}
	short res = (buf[0] << 8) + buf[1];
	if (res >= 32768){
		return -1*((65536 - res) + 1);
	}
	return res;
}
short readShortInv(int fd, int reg){

	unsigned char buf[16];
	buf[0]=reg;
	if (write(fd,buf,1) != 1){
		cout << "MPU6050: can't tell device to read from buffer" << endl;
		onerr = 10;
	}
	if (read(fd,buf,2) != 2){
		cout << "MPU6050: can't read from device" << endl;
		onerr = 10;
	}
	short res = (buf[1] << 8) + buf[0];
	if (res >= 32768){
		return -1*((65536 - res) + 1);
	}
	return res;
}
long long readLong(int fd, int reg, int bytes){

	unsigned char buf[16];
	buf[0]=reg;
	if (write(fd,buf,1) != 1){
		cout << "MPU6050: can't tell device to read from buffer" << endl;
		onerr = 10;
	}
	if (read(fd,buf,bytes) != bytes){
		cout << "MPU6050: can't read from device" << endl;
		onerr = 10;
	}
	long long res = 0;
	for (int i = 0; i < bytes; i++){
		res <<= 8;
		res += buf[i];
	}
	return res;
}
short readByte(int fd, int reg){

	unsigned char buf[16];
	buf[0]=reg;
	if (write(fd,buf,1) != 1){
		cout << "MPU6050: can't tell device to read from buffer" << endl;
		onerr = 10;
	}
	if (read(fd,buf,1) != 1){
		cout << "MPU6050: can't read from device" << endl;
		onerr = 10;
	}
	short res = buf[0];
	return res;
}

