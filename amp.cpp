#include "amp.h"
#include "i2c.h"

using namespace std;

int fd;
const int ADS1115_I2C_ADDR = 0x48


int main(){
  gpioInitialise();

    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        // Open port for reading and writing
        fprintf(stderr, "Failed to open i2c bus\n");

        //return 1;
    }

  selectDevice(fd, ADS1115_I2C_ADDR, "ADS1115");
  while (1)
    
    gpioPWM(4, i);
    //gpioWrite(4, 1);
    //usleep(1000000);
    //gpioWrite(4, 0);
    usleep(10000);
  }
}
