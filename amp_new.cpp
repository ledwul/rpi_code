#include "amp_new.h"
#include <current_control.h>

using namespace std;

const int I2C_ADDR = 0x48;
const int CONFIG = 0x01;
const int RESULT = 0x00;
const float SHUNT_OHMS = 0.50;
short val[4];

float getTarget(int light, float time){
    //if (light != 0) return 0;
    return 0.05 * (0.5 + 0.5* sin(2*M_PI*time/5 + 0.25*light * M_PI));
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

    ADS1115_ADC adc_handler(I2C_ADDR, CONFIG, RESULT, SHUNT_OHMS);
    ports_t ports = {4, 14, 15, 18};
    auto init = getTime();

    adc_handler.setOffsets(ports);

    int i = 0;
    float cur_trg = 0.08f;
    int index = 0;
    int steps = 500;
    for (int i = 0; i < 4; i++) gpioSetPWMrange(ports.at(i), steps);
    while (1) {
        index ++;
        float time = ((chrono::duration<float>)(getTime() - init)).count();
        for (int light = 0; light < 4; light++){
            float target = getTarget(light, time);
            //gpioPWM(port[light], val[light]);
            gpioPWM(ports.at(light), (int)(steps * 10 * target));
            float cur = adc_handler.getCurrent(light);
            if (cur < target && val[light] < steps || val[light] <= 0) val[light]++;
            else val[light]--;
            if (index % 100 == 0){
                cout << "Light " << light << ": " << cur << "A, " << adc_handler.getVoltage(light) - adc_handler.offsets_[light] << "V, Target: " << getTarget(light, time) << "A." << endl;
            }
        }
    }
    return 0;
}
