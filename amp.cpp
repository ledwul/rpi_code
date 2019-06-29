#include "amp.h"
#include "i2c.h"

using namespace std;

int fd;
const int ADS1115_I2C_ADDR = 0x48;
const int ADS1115_CONFIG = 0x01;
const int ADS1115_RESULT = 0x00;
const float SHUNT_OHMS = 0.50;
float offset[4];
short port[4];
short val[4];

std::chrono::high_resolution_clock::time_point getTime(){
  return chrono::high_resolution_clock::now();
}

float getVoltage(int input){

  long sum = 0;
  const int samples = 1;
  short ms;
  for (int i = 0; i <samples; i++){
  // 0: On, 1: Multiplex, 2: Gain, 3: Mode, 4: Rate, 5-8: Comparator
  //                                       0111222344456788  
  writeToDeviceShort(fd, ADS1115_CONFIG, 0b1100100111100011 | (input << 12));
  usleep(1200);
  ms = readShort(fd, ADS1115_RESULT);
  sum += ms;
  //cout << ms /32768.0 /samples * 0.512  - offset[i]<< "V "<<  i << endl;
  }
  return sum /32768.0 /samples * 0.512;
}

float getCurrent(int input)
{
  return (getVoltage(input) - offset[input]) / SHUNT_OHMS;
}

float getTarget(int light, float time){
  //if (light != 0) return 0;
  return 0.05 * (0.5 + 0.5* sin(2*M_PI*time/5 + 0.25*light * M_PI));
}

int main(){
  gpioInitialise();

    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0) {
        // Open port for reading and writing
        fprintf(stderr, "Failed to open i2c bus\n");
	
        //return 1;
    }

  selectDevice(fd, ADS1115_I2C_ADDR, "ADS1115");

  port[0]=4;
  port[1]=14;
  port[2]=15;
  port[3]=18;

  auto init = getTime();
  
  for (int i = 0; i < 4; i++){
    gpioWrite(port[i], 0);
    usleep(100000);
    offset[i] = getVoltage(i);
    cout << "offset of " << i << " is " << offset[i] << "V."<<endl;
  }

  int i = 0;
  float cur_trg = 0.08f;
  int index = 0;
  int steps = 500;
  for (int i = 0; i < 4; i++) gpioSetPWMrange(port[i], steps);
  while (1) {
    index ++;
    float time = ((chrono::duration<float>)(getTime() - init)).count();
    for (int light = 0; light < 4; light++){
      float target = getTarget(light, time);
      //gpioPWM(port[light], val[light]);
      gpioPWM(port[light], (int)(steps * 10 * target));
      float cur = getCurrent(light);
      if (cur < target && val[light] < steps || val[light] <= 0) val[light]++;
      else val[light]--;
      if (index % 100 == 0){
	cout << "Light " << light << ": " << cur << "A, " << getVoltage(light) - offset[light] << "V, Target: " << getTarget(light, time) << "A." << endl;
      }
    }
  }
}
