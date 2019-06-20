#include "amp.h"
#include "i2c.h"

using namespace std;

int fd;
const int ADS1115_I2C_ADDR = 0x48;
const int ADS1115_CONFIG = 0x01;
const int ADS1115_RESULT = 0x00;

int main(){
  gpioInitialise();

    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        // Open port for reading and writing
        fprintf(stderr, "Failed to open i2c bus\n");
	
        //return 1;
    }

  selectDevice(fd, ADS1115_I2C_ADDR, "ADS1115");
  
  while (1) {
    // 0: On, 1: Multiplex, 2: Gain, 3: Mode, 4: Rate, 5-8: Comparator
    //                                       0111222344456788
    writeToDeviceShort(fd, ADS1115_CONFIG, 0b1100100110000011);
    
    gpioPWM(4, 220);
    //gpioWrite(4, 1);
    //usleep(1000000);
    //gpioWrite(4, 0);
    usleep(1000000);
    short res = readShort(fd, ADS1115_RESULT);
    cout << res << endl;
    float voltage = res /32768.0 * 0.512;
    cout << voltage << "V" << endl;
  }
}
