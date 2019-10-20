#include "amp_new.h"
#include <current_control.h>

using namespace std;

const int I2C_ADDR = 0x48;
const float SHUNT_OHMS = 0.50;
short val[4];

float getTarget(int light, float time){
  //if (light != 0) return 0;
  //return 0.3 * (0.5 + 0.5* (sin(2*M_PI*time/50 + 0.25*(light%2) * M_PI)));
  const float target_brightness = 0.3;
  if (time > 0) return target_brightness;
  float ramp_time ;
  if (light % 2 == 0){
    ramp_time = 10;
  }else{
    ramp_time = 20;
  }
  if (time < -ramp_time) return 0;
  return target_brightness * (ramp_time + time) / ramp_time;
}

std::chrono::high_resolution_clock::time_point getTime(){
  return chrono::high_resolution_clock::now();
}

int main(){
  try{
    if(gpioInitialise() < 0) throw 1;
  }
  catch (int e) {
    cout << "GPIO pins could not be initialized" << endl;
  }

  
  ifstream myfile ("wakeuptime.txt");
  int target_time;
  if (myfile.is_open()){
    myfile >> target_time;
  }else{
    cerr << "wakeuptime.txt not found!" << endl;
    target_time = time(0) + 25;
  }
  int time_until_wakeup = target_time - time(0);
  
  ADS1115_ADC adc_handler(I2C_ADDR, SHUNT_OHMS);
  ports_t ports = {4, 14, 15, 18};
  vector<float> curs(4);
  auto init = getTime();

  adc_handler.setOffsets(ports);

  float cur_trg = 0.08f;
  int index = 0;
  short steps = 30000;
  for (int i = 0; i < 4; i++) {
    gpioSetPWMrange(ports.at(i), steps);
    gpioPWM(ports[i], 0);
  }
  while (1) {
    index ++;
    float time = ((chrono::duration<float>)(getTime() - init)).count() - time_until_wakeup;
    for (int light = 0; light < 4; light++){
      float target = getTarget(light, time);
      //gpioPWM(ports.at(light), (int)(steps * 10 * target));
      curs[(light-1+curs.size())%curs.size()] = adc_handler.getCurrent(light);
      float cur = curs[light];
      if (target < 0.01){
	val[light] = 0;
      }else{
	const float P = 0.01;
	int delta = lround(P * (target - cur)*steps);
	val[light] += delta;
	const int maxdelta = steps / 1000;
	if (val[light] > steps+maxdelta) val[light]=steps+maxdelta;
	if (val[light] < -maxdelta) val[light]=-maxdelta;
      }
      gpioPWM( ports[light], max((short)0, min( steps,val[light])));
      if (index % 1000 == 0){
	cout << "Light " << light << ": " << cur << "A (" << val[light]<<"), " << adc_handler.getVoltage(light) - adc_handler.offsets_[light] << "V, Target: " << getTarget(light, time) << "A." << endl;
      }
    }
  }
  return 0;
}
